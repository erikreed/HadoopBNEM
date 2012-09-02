#!/bin/bash
# erik reed

mkdir -p results

trials=1000
iters=15
pop=10
export OMP_NUM_THREADS=4

# asia tests -- 8 nodes, 2hrs
net=asia
for i in `seq 1 1 $trials`; do
	for samp in 100 200 400 800 1600; do
		for hid in 2 4 6; do
			echo $net: samples=$samp, hidden=$hid, $i of $trials
			./scripts/init.sh dat/bnets/$net.net $samp $hid
			./scripts/test_mapreduce.sh dat/in/ $iters $pop -u > results/$net.$samp.$hid.$i.txt
			./scripts/test_mapreduce.sh dat/in/ $iters $pop -alem > results/$net.$samp.$hid.$i.alem.txt
		done
	done
done
7za a $net.results.7z results
rm -rf results
