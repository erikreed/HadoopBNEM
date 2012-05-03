#!/bin/bash -e
# erik reed
# runs EM algorithm on hadoop streaming

# expects: fg,em,tab files
DAT_DIR=dat_small

# max MapReduce job iterations, not max EM iters
MAX_ITERS=100 

REDUCERS=1 # TODO: bug when REDUCERS > 1

# 2 choices: -u and -alem
# -u corresponds to update; standard EM with fixed population size
#		e.g. a simple random restart with $POP BNs
# -alem corresponds to Age-Layered EM with dynamic population size
EM_FLAGS="-alem"

# set to min_runs[0]
POP=1
MAPPERS=2

echo -- Using parameters -- 
echo Directory: $DAT_DIR
echo Max number of MapReduce iterations: $MAX_ITERS
echo Reducers: $REDUCERS
echo Mappers: $MAPPERS
echo Population size: $POP
#echo BNs per mapper: $(($POP / $MAPPERS))
echo ----------------------
./scripts/make_input.sh $DAT_DIR
rm -rf out
mkdir -p out
hadoop fs -rmr out in || true
hadoop fs -put in in || true

echo $POP > in/pop

# randomize initial population
names=
for id in $(seq 0 1 $(($POP - 1)))
do
	names+="in/dat.$id "
done
./utils in/fg $names

# copy 0th iteration (i.e. initial values)
mkdir -p out/iter.0
cp in/dat.* out/iter.0


for i in $(seq 1 1 $MAX_ITERS); do
	echo starting MapReduce job iteration: $i
	# ASD used because * delimeter is removed; need to tweak reducer
	$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/contrib/streaming/hadoop-streaming-1.0.0.jar \
		-files "./dai_map,./dai_reduce,./in" \
		-D 'stream.map.output.field.separator=ASD' \
		-D 'stream.reduce.output.field.separator=ASD' \
		-D mapred.tasktracker.tasks.maximum=$MAPPERS \
		-D mapred.map.tasks=$MAPPERS \
		-D dfs.block.size=256KB \
		-input ./in/tab_content \
		-output out \
		-mapper ./dai_map \
		-reducer ./dai_reduce \
		-numReduceTasks $REDUCERS

	hadoop fs -get out/part-00000 out/tmp
	hadoop fs -rmr out
	cat out/tmp | ./utils $EM_FLAGS
	rm out/tmp
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

