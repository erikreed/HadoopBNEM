$HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/hadoop-streaming.jar \
    -input in/tab_content \
    -output out \
    -mapper ./dai_map \
    -reducer ./dai_reduce \
	-numReduceTasks 1 \
	-file ./dai_map \
	-file ./dai_reduce \
	-file in/tab_header \
	-D 'stream.map.output.field.separator=*'
	
