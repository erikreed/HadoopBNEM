hadoop fs -rmr out

$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/contrib/streaming/hadoop-streaming-1.0.0.jar \
	-files "./dai_map,./dai_reduce,./in/tab_header,./in" \
	-D 'stream.map.output.field.separator=ASD' \
	-D 'stream.reduce.output.field.separator=ASD' \
    -input ./in/tab_content \
    -output out \
    -mapper ./dai_map \
    -reducer ./dai_reduce \
	-numReduceTasks 1 
	
