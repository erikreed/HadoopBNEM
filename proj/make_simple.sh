#erik reed
# try -fwhole-program
g++ -fwhole-program -fprofile-use -flto -I/data01/Projects/David_and_Erik/bullshit/gmp-5.0.4/bin -fprofile-dir=profiling -ffast-math -ftree-vectorize -mtune=native -march=native -fopenmp -I../include -Wno-deprecated -Wall -W -Wextra -O3 -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -osimple/simple  ../*.o  simple/simple.cpp -lgmp -lgmpxx -Wl,-Bdynamic

