#erikreed@cmu.edu

OPT_FLAGS= -Wall -O3 -ffast-math -fwhole-program -flto

DAI_FLAGS=$(OPT_FLAGS) -DDAI_WITH_JTREE -DDAI_WITH_GIBBS 
DAI_FLAGS+= -DDAI_DEBUG
export PLATFORM=Linux-amd64-64
export HADOOP_INSTALL=/home/erik/hadoop/hadoop-1.0.0

INCLUDE=-L${HADOOP_HOME}/c++/Linux-amd64-64/lib \
	-I${HADOOP_HOME}/c++/Linux-amd64-64 

LIBS=-Wl,-Bstatic -lgmpxx -lm -lc -lgmp -lboost_serialization \
    -lz -lboost_iostreams -Wl,-Bdynamic -lpthread 

all: dai_map dai_reduce utils 

.PHONY: run clean hadoop mvjar cleanout clobber
clobber:
	rm -rf tmp $(shell echo /tmp/hadoop-`whoami`*) in out dat/{in,out}

cleanout: 
	hadoop fs -rmr out 

clean:
	rm -f dai_map dai_reduce utils alem

run: all
	./streaming.sh

dai_map: src/dai_map.cpp ../lib/libdai.a src/dai_mapreduce.h
	g++ -Wno-write-strings -I../include -I. $(DAI_FLAGS)  \
    -odai_map src/dai_map.cpp ../lib/libdai.a $(LIBS)

dai_reduce: src/dai_reduce.cpp ../lib/libdai.a src/dai_mapreduce.h
	g++ -Wno-write-strings -I../include -I. $(DAI_FLAGS)  \
    -odai_reduce src/dai_reduce.cpp ../lib/libdai.a $(LIBS)

utils: src/utils.cpp ../lib/libdai.a src/dai_mapreduce.h
	g++ -Wno-write-strings -fopenmp -I../include -I. $(DAI_FLAGS)  \
    -outils src/utils.cpp ../lib/libdai.a $(LIBS)

# interesting: must use -O3 on main compilation for -static to compile successfully
alem: src/alem.cpp ../lib/libdai.a src/dai_mapreduce.h
	g++ -Wno-write-strings -fopenmp -I../include -I. $(DAI_FLAGS)  \
    -oalem src/alem.cpp ../lib/libdai.a $(LIBS)


