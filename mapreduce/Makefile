#erikreed@cmu.edu

OPT_FLAGS= -Wall -O3 -ffast-math -fwhole-program -flto

DAI_FLAGS=$(OPT_FLAGS) -DDAI_WITH_JTREE -DDAI_WITH_GIBBS 
DAI_FLAGS+= -DDAI_DEBUG
export PLATFORM=Linux-amd64-64
export HADOOP_INSTALL=/home/erik/hadoop/hadoop-1.0.0

INCLUDE=-L${HADOOP_HOME}/c++/Linux-amd64-64/lib \
	-I${HADOOP_HOME}/c++/Linux-amd64-64 
LIBS=-lhadooppipes -lhadooputils -lgmpxx -lm -lc -lgmp \
	-lssl -lcrypto -lboost_serialization \
	-lpthread -lz -ldl

LIBS2=-lgmpxx -lm -lc -lgmp \
    -lboost_serialization -lpthread

all: dai_map dai_reduce utils alem 

.PHONY: run clean hadoop mvjar cleanout clobber
clobber:
	rm -rf tmp /tmp/hadoop-erik* in out

cleanout: 
	hadoop fs -rmr out 

clean:
	rm -f dai_map dai_reduce utils alem

run: all
	./streaming.sh

dai_map: src/dai_map.cpp ../lib/libdai.a src/dai_mapreduce.h
	g++ -static -Wno-write-strings -I../include -I. $(DAI_FLAGS)  \
    -odai_map src/dai_map.cpp ../lib/libdai.a $(LIBS2)

dai_reduce: src/dai_reduce.cpp ../lib/libdai.a src/dai_mapreduce.h
	g++ -static -Wno-write-strings -I../include -I. $(DAI_FLAGS)  \
    -odai_reduce src/dai_reduce.cpp ../lib/libdai.a $(LIBS2)

utils: src/utils.cpp ../lib/libdai.a src/dai_mapreduce.h
	g++ -static -Wno-write-strings -fopenmp -I../include -I. $(DAI_FLAGS)  \
    -outils src/utils.cpp ../lib/libdai.a $(LIBS2)

alem: src/alem.cpp ../lib/libdai.a src/dai_mapreduce.h
	g++ -static -Wno-write-strings -fopenmp -I../include -I. $(DAI_FLAGS)  \
    -oalem src/alem.cpp ../lib/libdai.a $(LIBS2)


