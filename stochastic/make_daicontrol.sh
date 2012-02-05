JAVA1=/usr/lib64/jvm/java-1.6.0-openjdk-1.6.0/include/linux
JAVA2=/usr/lib64/jvm/java-1.6.0-openjdk-1.6.0/include
g++ -fopenmp -I../include -I. -fpic -g -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -odaicontrol daicontrol.cpp ../lib/libdai.a ../lib/libdai.a -lgmpxx -lgmp \
	-I$JAVA1 -I$JAVA2 && echo done making executable &

g++ -shared -fPIC -fopenmp -I../include -I. -fpic -g -DDAI_WITH_BP -DDAI_WITH_FBP -DDAI_WITH_TRWBP -DDAI_WITH_MF -DDAI_WITH_HAK -DDAI_WITH_LC -DDAI_WITH_TREEEP -DDAI_WITH_JTREE -DDAI_WITH_MR -DDAI_WITH_GIBBS -DDAI_WITH_CBP -DDAI_WITH_DECMAP -oDaiControl.so daicontrol.cpp ../lib/libdai.a ../lib/libdai.a -lgmpxx -lgmp \
	-I$JAVA1 -I$JAVA2 && echo done making shared library &
