./make_input.sh
rm -rf out
mkdir out
hadoop fs -rmr out in
hadoop fs -put in in

ITERS=2
MAPPERS=4
REDUCERS=1 # bug when REDUCERS > 1
POP=5

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

for i in $(seq 1 1 $ITERS); do
	echo starting iteration number: $i
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
	cat out/tmp | ./utils -u
	rm out/tmp
	mkdir -p out/iter.$i
	cp out/dat.* in # overwrite previous iteration
	mv out/dat.* out/iter.$i
done

