#!/bin/bash -e
# erik reed
# runs EM algorithm on hadoop streaming

if [ $# -ne 5 ]; then
    echo Usage: $0 \"dir with net,tab,fg,em\" EM_FLAGS MAPPERS REDUCERS POP
    echo "2 choices for EM_FLAGS: -u and -alem"
    echo "  -u corresponds to update; standard EM with fixed population size"
    echo "    e.g. a simple random restart with $POP BNs"
    echo "  -alem corresponds to Age-Layered EM with dynamic population size"
    exit 1
fi

DAT_DIR=$1
EM_FLAGS=$2
MAPPERS=$3
REDUCERS=$4
# set to min_runs[0] if using ALEM
POP=$5

HADOOP_JAR=`echo $HADOOP_PREFIX/contrib/streaming/*.jar`

if [ -n "$HADOOP_PREFIX" ]; then
  HADOOP_HOME=$HADOOP_PREFIX
elif [ -n "$HADOOP_HOME" ]; then
  echo Hadoop env vars not proeprly set!
  exit 1
else
  HADOOP_PREFIX=$HADOOP_HOME
fi

# max MapReduce job iterations, not max EM iters, which
# is defined in dai_mapreduce.h

MAX_ITERS=1

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
echo EM flags: $EM_FLAGS
echo Max MR iterations: $MAX_ITERS
echo ---------------------- | $LOG

./scripts/make_input.sh $DAT_DIR
echo $POP > in/pop

# create random initial population
./utils in/fg $POP

# copy 0th iteration (i.e. initial values)
mkdir -p dat/out/iter.0
cp dat/in/dat dat/out/iter.0 
cp dat/in/dat in

echo Clearing previous input from HDFS
hadoop fs -rmr -skipTrash out in &> /dev/null || true 
echo Adding input to HDFS
hadoop fs -D dfs.replication=1 -put in in

for i in $(seq 1 1 $MAX_ITERS); do
  echo starting MapReduce job iteration: $i
  $HADOOP_PREFIX/bin/hadoop jar $HADOOP_JAR \
    -files "dai_map,dai_reduce,in" \
    -D 'stream.map.output.field.separator=:' \
    -D 'stream.reduce.output.field.separator=:' \
    -D mapred.tasktracker.tasks.maximum=$MAPPERS \
    -D mapred.map.tasks=$MAPPERS \
    -D mapred.output.compress=false \
    -input in/tab_content \
    -output out \
    -mapper ./dai_map \
    -reducer ./dai_reduce \
    -numReduceTasks $REDUCERS
  hadoop fs -cat out/part-* > dat/out/tmp
  hadoop fs -rmr -skipTrash out in/dat
  cat dat/out/tmp | ./utils $EM_FLAGS
  rm dat/out/tmp in/dat # remove previous iteration
  mkdir -p dat/out/iter.$i
  mv out/* dat/out/iter.$i
  
  if [ $i != $MAX_ITERS ]; then
    cp out/dat in
    echo Adding next iteration input to HDFS
    hadoop fs -D dfs.replication=1 -put out/dat in
  fi

  if [ -n "$start_time" -a -n "$time_duration" ]; then
    dur=$((`date +%s` - $start_time))
    if [ $dur -ge $time_duration ]; then
      echo $time_duration seconds reached! Quitting...
      break
    fi     
  fi

  converged=`cat dat/out/iter.$i/converged`
  if [ "$converged" = 1 ]; then
    echo EM converged at iteration $i
    break
  fi
done

