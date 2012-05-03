#!/bin/bash -e
# erik reed
# runs EM algorithm on hadoop streaming

# if using Amazon EC2 Linux AMI
HADOOP_JAR=hadoop-0.19.0-streaming.jar
#HADOOP_JAR=hadoop-streaming-1.0.0.jar

# expects: fg,em,tab files
DAT_DIR=dat_small

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
rm -rf out
mkdir -p out

LOG="tee -a out/log.txt"
echo $0 started at `date`  | $LOG
echo -- Using parameters -- | $LOG
echo Directory: $DAT_DIR | $LOG
echo Max number of MapReduce iterations: $MAX_ITERS | $LOG
echo Reducers: $REDUCERS | $LOG
echo Mappers: $MAPPERS | $LOG
echo Population size: $POP | $LOG
#echo BNs per mapper: $(($POP / $MAPPERS))
echo ---------------------- | $LOG
./scripts/make_input.sh $DAT_DIR

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
	$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/contrib/streaming/$HADOOP_JAR \
		-files "./dai_map,./dai_reduce,./in" \
		-D 'stream.map.output.field.separator=ASD' \
		-D 'stream.reduce.output.field.separator=ASD' \
		-D mapred.tasktracker.tasks.maximum=$MAPPERS \
		-D mapred.map.tasks=$MAPPERS \
		-D mapred.output.compress=false \
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

