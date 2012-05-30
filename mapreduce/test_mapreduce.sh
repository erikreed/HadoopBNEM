#!/bin/bash -e
# erik reed
# runs EM algoritm on MapReduce locally

# expects: fg,em,tab files
if [ $# -ne 1 ]; then                                                                         
    echo Usage: $0 \"dir with net,fg,em\"
    exit 1                                                                                    
fi                                                                                            
                                                                                              
DAT_DIR=$1

# max MapReduce job iterations, not max EM iters
MAX_ITERS=5

REDUCERS=1 # TODO: bug when REDUCERS > 1

# 2 choices: -u and -alem
# -u corresponds to update; standard EM with fixed population size
#		e.g. a simple random restart with $POP BNs
# -alem corresponds to Age-Layered EM with dynamic population size
EM_FLAGS="-u"

# set to min_runs[0]
POP=100

# (disabled) save previous run if it exists (just in case)
#rm -rf dat/out.prev
#mv -f dat/out dat/out.prev || true
rm -rf out
mkdir -p out

LOG="tee -a out/log.txt"
echo $0 started at `date`  | $LOG
echo -- Using parameters -- | $LOG
echo Directory: $DAT_DIR | $LOG
echo Max number of MapReduce iterations: $MAX_ITERS | $LOG
echo Population size: $POP | $LOG
echo EM flags: $EM_FLAGS
echo ---------------------- | $LOG
./scripts/make_input.sh $DAT_DIR

echo $POP > dat/in/pop

# randomize initial population
names=
for id in $(seq 0 1 $(($POP - 1)))
do
	names+="dat/in/dat.$id "
done
./utils dat/in/fg $names

cp dat/in/* in

# copy 0th iteration (i.e. initial values)
mkdir -p out/iter.0
cp in/dat.* out/iter.0

for i in $(seq 1 1 $MAX_ITERS); do
	echo starting local MapReduce job iteration: $i
	
	cat in/tab_content | ./dai_map | ./dai_reduce | ./utils $EM_FLAGS

	mkdir -p out/iter.$i
	rm in/dat.* # remove previous iteration
	cp out/dat.* in 
	mv out/dat.* out/iter.$i

	converged=`cat out/converged`
	if [ "$converged" = 1 ]; then
		echo EM complete!
		break
	fi
done

