#!/bin/bash -e
# erik reed
# runs EM algorithm on hadoop streaming

if [ $# -ne 4 ]; then
    echo Usage: $0 \"dir with net,tab,fg,em\" EM_FLAGS MAPPERS POP
    echo "2 choices for EM_FLAGS: -u and -alem"
    echo "  -u corresponds to update; standard EM with fixed population size"
    echo "    e.g. a simple random restart with $POP BNs"
    echo "  -alem corresponds to Age-Layered EM with dynamic population size"
    exit 1
fi

DAT_DIR=$1
EM_FLAGS=$2
MAPPERS=$3
# set to min_runs[0]
POP=$4

# if using Amazon EC2 Linux AMI
#  HADOOP_JAR=hadoop-0.19.0-streaming.jar
# else
HADOOP_JAR=hadoop-streaming-1.0.0.jar
# endif

# max MapReduce job iterations, not max EM iters, which
# is defined in dai_mapreduce.h

MAX_ITERS=50

REDUCERS=1 # TODO: bug when REDUCERS > 1



# (disabled) save previous run if it exists (just in case)
#rm -rf out.prev
#mv -f out out.prev || true
rm -rf dat/out out
mkdir -p dat/out out

LOG="tee -a dat/out/log.txt"
echo $0 started at `date`  | $LOG
echo -- Using parameters -- | $LOG
echo Directory: $DAT_DIR | $LOG
echo Max number of MapReduce iterations: $MAX_ITERS | $LOG
echo Reducers: $REDUCERS | $LOG
echo Mappers: $MAPPERS | $LOG
echo Population size: $POP | $LOG
#echo BNs per mapper: $(($POP / $MAPPERS))
echo EM flags: $EM_FLAGS
echo ---------------------- | $LOG
./scripts/make_input.sh $DAT_DIR

echo $POP > in/pop

# randomize initial population
names=
for id in $(seq 0 1 $(($POP - 1)))
do
	names+="dat/in/dat.$id "
done
./utils in/fg $names

# copy 0th iteration (i.e. initial values)
mkdir -p dat/out/iter.0
cp dat/in/dat.* dat/out/iter.0 
cp dat/in/dat.* in

hadoop fs -rmr out in || true
hadoop fs -put in in

for i in $(seq 1 1 $MAX_ITERS); do
	echo starting MapReduce job iteration: $i
	# ASD used because * delimeter is removed; need to tweak reducer
	$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/contrib/streaming/$HADOOP_JAR \
		-files "dai_map,dai_reduce,in" \
		-D 'stream.map.output.field.separator=ASD' \
		-D 'stream.reduce.output.field.separator=ASD' \
		-D mapred.tasktracker.tasks.maximum=$MAPPERS \
		-D mapred.map.tasks=$MAPPERS \
		-D mapred.output.compress=false \
		-D dfs.block.size=256KB \
		-input in/tab_content \
		-output out \
		-mapper ./dai_map \
		-reducer ./dai_reduce \
		-numReduceTasks $REDUCERS

	hadoop fs -get out/part-00000 dat/out/tmp
	hadoop fs -rmr out in/dat.*
	cat dat/out/tmp | ./utils $EM_FLAGS
	rm dat/out/tmp in/dat.* # remove previous iteration
	mkdir -p dat/out/iter.$i
	cp out/dat.* in
	hadoop fs -put out/dat.* in
	mv out/* dat/out/iter.$i

	converged=`cat dat/out/iter.$i/converged`
	if [ "$converged" = 1 ]; then
		echo EM converged at iteration $i
		break
	fi
done

