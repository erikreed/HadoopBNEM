#!/bin/bash -e
# erik reed
# runs EM algorithm on hadoop streaming

if [ $# -ne 1 ]; then
    echo Usage: $0 \"dir with net,fg,em\"
    exit 1
fi

DAT_DIR=$1
# if using Amazon EC2 Linux AMI
HADOOP_JAR=hadoop-0.19.0-streaming.jar
#HADOOP_JAR=hadoop-streaming-1.0.0.jar

# expects: fg,em,tab files


# max MapReduce job iterations, not max EM iters
MAX_ITERS=100

REDUCERS=1 # TODO: bug when REDUCERS > 1

# 2 choices: -u and -alem
# -u corresponds to update; standard EM with fixed population size
#		e.g. a simple random restart with $POP BNs
# -alem corresponds to Age-Layered EM with dynamic population size
EM_FLAGS="-u"

# set to min_runs[0]
POP=10
MAPPERS=10

# (disabled) save previous run if it exists (just in case)
#rm -rf out.prev
#mv -f out out.prev || true
rm -rf dat/out
mkdir -p dat/out

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

hadoop fs -rmr out in || true
hadoop fs -put in in || true

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


for i in $(seq 1 1 $MAX_ITERS); do
	echo starting MapReduce job iteration: $i
	# ASD used because * delimeter is removed; need to tweak reducer
	$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/contrib/streaming/$HADOOP_JAR \
		-files "./dai_map,./dai_reduce,./in" \
		-D 'stream.map.output.field.separator=ASD' \
		-D 'stream.reduce.output.field.separator=ASD' \
		-D mapred.tasktracker.tasks.maximum=$MAPPERS \
		-D mapred.map.tasks=$MAPPERS \
		-D mapred.output.compress=false \
		-D dfs.block.size=256KB \
		-input dat/in/tab_content \
		-output out \
		-mapper ./dai_map \
		-reducer ./dai_reduce \
		-numReduceTasks $REDUCERS

	hadoop fs -get out/part-00000 dat/out/tmp
	hadoop fs -rmr dat/out
	cat dat/out/tmp | ./utils $EM_FLAGS
	rm dat/out/tmp
	mkdir -p dat/out/iter.$i
	rm dat/in/dat.* # remove previous iteration
	cp dat/out/dat.* dat/in 
	mv dat/out/dat.* dat/out/iter.$i

	converged=`cat dat/out/converged`
	if [ "$converged" = 1 ]; then
		echo EM complete!
		break
	fi
done

