# Include flags
INC=-I../include
# Library path flags
LIBS=-lgmpxx -lgmp
# Location of libDAI library
LIB=../lib
# Compiler
CC=g++
# Compiler flags
CCFLAGS=-Wno-deprecated -Wall -W -Wextra -fpic -O3 -static $(INC)

all : uai2010-aie-solver asia simple


#uai2010-aie-solver : uai2010-aie-solver.cpp $(LIB)/libdai.a
#	$(CC) $(CCFLAGS) -o$@ $< $(LIB)/libdai.a $(LIBS)


sprinkler : sprinkler/sprinkler.cpp $(LIB)/libdai.a
	                g++ -I../include -Wno-deprecated -Wall -W -Wextra -fpic -O3 -g -DDAI_DEBUG  -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -osprinkler/$@ $< ../lib/libdai.a -lgmpxx -lgmp

simple : simple/simple.cpp $(LIB)/libdai.a
	                g++ -I../include -Wno-deprecated -Wall -W -Wextra -fpic  -g -DDAI_DEBUG  -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -osimple/$@ $< ../lib/libdai.a -lgmpxx -lgmp


uai2010-aie-solver : uai2010-aie-solver.cpp $(LIB)/libdai.a
	g++ -I../include -Wno-deprecated -Wall -W -Wextra -fpic -O3 -g -DDAI_DEBUG  -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -o$@ $< ../lib/libdai.a -lgmpxx -lgmp


asia : asia.cpp $(LIB)/libdai.a
	        g++ -I../include -Wno-deprecated -Wall -W -Wextra -fpic -O3 -g -DDAI_DEBUG  -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -o$@ $< ../lib/libdai.a -lgmpxx -lgmp

# working g++
# g++ -I../include -Wno-deprecated -Wall -W -Wextra -fpic -O3 -g -DDAI_DEBUG  -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP uai2010-aie-solver.cpp ../lib/libdai.a -lgmpxx -lgmp
#


# CLEAN
########

.PHONY : clean run
run: simple
	./simple/simple ./simple/simple.fg ./simple/simple.tab

clean :
	-rm -f uai2010-aie-solver asia simple/simple sprinker/sprinkler
