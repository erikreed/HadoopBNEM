#erikreed@cmu.edu

all: param_sharing

param_sharing: src/param_sharing.cpp
	g++ -O3  -I../include -I. -g -DDAI_WITH_JTREE -DDAI_WITH_GIBBS -oparam_sharing src/param_sharing.cpp ../lib/libdai.a ../lib/libdai.a -lgmpxx -lgmp 


