#erikreed@cmu.edu

all: param_sharing

.PHONY: clean
clean:
	rm -f param_sharing

param_sharing: src/param_sharing.cpp
	g++ -O3 -fopenmp -march=native -mtune=native -fwhole-program -ffast-math -fomit-frame-pointer  -I../include -I. -g -DDAI_WITH_JTREE -DDAI_WITH_GIBBS -oparam_sharing src/param_sharing.cpp ../lib/libdai.a ../lib/libdai.a -lgmpxx -lgmp 


