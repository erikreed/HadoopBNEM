#!/bin/bash

# this script fixes the number of hidden nodes and increasing num_shared
# are subsets of eachother

DEST=same_subset_results
SHARED=12
#BNETS="adapt_mini1 adapt_subset2_large adapt10_v7e \
	#		diabetes link mildew pigs water"
BNETS="ADAPT_PHM09_T1"  #pigs water link adapt10_v7e mildew diabetes"
BNET_DIR=bnets

export NUM_SAMPLES=5000

echo Using $BNETS

export OMP_NUM_THREADS=50 #uncomment for maxthreads (64)
echo Using $OMP_NUM_THREADS threads


export FIXED_NODES=""
for net in $BNETS
do
	# note this only returns shareds >= 2 
	shared=`python net_to_fg/read_net_erik.py $BNET_DIR/$net.net`
	shared_sorted=`echo $shared | tr " " "\n" | sort -g -r`
	echo set of possible shared params for $net: $shared_sorted
	for s in $shared_sorted
	do
		if [ $SHARED -eq $s ]; then
			sharedvars=`python net_to_fg/read_net_erik.py $BNET_DIR/$net.net $s`
			echo using hidden nodes: $sharedvars
			for j in $(seq 2 2 $SHARED)
			do
				svars=`echo $sharedvars |  tr " " "\n" | head -$j`
				echo using shared nodes: $svars

				export HIDDEN_NODES=$sharedvars
				export SHARED_NODES=$svars
				./master.sh $BNET_DIR/$net.net
				mkdir -p $DEST/$net
				mv out $DEST/$net/s$j

				export SHARED_NODES=''
				./master.sh $BNET_DIR/$net.net
				mv out $DEST/$net/h$j
			done
		fi
	done
	echo completed $net
done

