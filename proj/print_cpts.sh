#!/bin/bash

DEST=cpt_counts
MIN_SHARED=1
BNETS="ADAPT_PHM09_T1 ADAPT_PHM09_T2 ADAPT_PHM10_P1 ADAPT_PHM10_P2 adapt_subset2_large alarm asia diabetes link mildew pigs water"
BNET_DIR=bnets

mkdir -p cpt_counts

for net in $BNETS
do
	shared=`python net_to_fg/read_net_erik.py $BNET_DIR/$net.net`
	shared_sorted=`echo $shared | tr " " "\n" | sort -g -r`
	echo $net \($shared_sorted\)
	echo num_hidden,cpt_size,num_parents > $DEST/$net.csv
	for s in $shared_sorted
	do
		if (( MIN_SHARED <= s )); then

			sharedvars=`python net_to_fg/read_net_erik.py $BNET_DIR/$net.net $s`
			cpt=`python net_to_fg/CPT_dim.py dat/$net.dat $sharedvars`
			
			# num hidden, CPT size, num parents
			echo `python print_cpts.py $s $cpt` >> $DEST/$net.csv

		fi
	done
	echo completed $net
done

