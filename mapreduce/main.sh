#!/bin/bash
# erik reed

# num_samples, num_hidden

mkdir -p results

trials=50
iters=15
pop=10

# asia tests -- 8 nodes, 2hrs
net=asia
for samp in 100 200 400 800 1600; do
	for hid in 2 4 6; do
		for i in `seq 1 1 $trials`; do
			echo $net: samples=$samp, hidden=$hid, $i of $trials
			./scripts/init.sh dat/bnets/$net.net $samp $hid
			./scripts/test_mapreduce.sh dat/in/ $iters $pop -u > results/$net.$samp.$hid.$i.txt
			./scripts/test_mapreduce.sh dat/in/ $iters $pop -alem > results/$net.$samp.$hid.$i.alem.txt
		done
	done
done

