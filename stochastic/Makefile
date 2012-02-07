#erikreed@cmu.edu
JAVA1=/usr/lib64/jvm/java-1.6.0-openjdk-1.6.0/include/linux
JAVA2=/usr/lib64/jvm/java-1.6.0-openjdk-1.6.0/include


all: DaiControl.class libdaicontrol.so 

lib.jar: hadoop/DaiReduce2.java
	~/hadoop/apache-ant-1.8.2/bin/ant -f hadoop/make_jar.xml

hadoop: libdaicontrol.so lib.jar
	#hadoop fs -copyFromLocal libdaicontrol.so /libraries/libdaicontrol.so && 
	rm -rf DaiReduce2dai_temp && \
		hadoop jar lib.jar -files libdaicontrol.so -Djava.library.path=. 3 3

.PHONY: run clean hadoop

clean:
	rm -f *.log *.o *.so daicontrol *.class DaiControl.h
	rm -rf DaiReduce2dai_temp

run: all
	java -Djava.library.path=. DaiControl

DaiControl.class: DaiControl.java
	javac DaiControl.java 
	
DaiControl.h: DaiControl.class
	javah -jni DaiControl

daicontrol: daicontrol.cpp DaiControl.class DaiControl.h
	g++ -O3 -fopenmp -I../include -I. -fPIC -g -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -odaicontrol daicontrol.cpp ../lib/libdai.a ../lib/libdai.a -lgmpxx -lgmp \
		        -I$(JAVA1) -I$(JAVA2)

libdaicontrol.so: daicontrol.cpp DaiControl.class DaiControl.h
	g++ -O3 -shared -Wall -fPIC -fopenmp -I../include -I. -g -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -olibdaicontrol.so daicontrol.cpp ../lib/libdai.a ../lib/libdai.a -lgmpxx -lgmp \
		        -I$(JAVA1) -I$(JAVA2)

