#erikreed@cmu.edu
# change lib64 to lib on ubuntu

DAI_FLAGS=-DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP

LIBS=-lgmpxx -lgmp -lboost_serialization

all: daicontrol

mvdat: 
	hadoop fs -copyFromLocal dat dat

.PHONY: run clean hadoop mvjar mvdat

clean:
	rm -f *.log *.o *.so *.a *.jar daicontrol *.class DaiControl.h && \
	rm -rf DaiReduce2dai_temp tmp /tmp/hadoop-erik*

run: all
	./daicontrol

daicontrol: daicontrol.cpp ../lib/libdai.a
	g++ -Wall -g -Wno-write-strings -I../include -I. $(DAI_FLAGS) -odaicontrol \
		daicontrol.cpp ../lib/libdai.a $(LIBS) 

