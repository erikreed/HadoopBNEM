#!/bin/bash
# erik reed

# num_samples, num_hidden

mkdir -p results

# asia tests
net=asia
for samp in 500 1000 2000 4000 8000 16000 32000; do
	for hid in 5 10 15 20 25; do
		echo $net: samples=$samp, hidden=$hid
		./scripts/init.sh dat/bnets/$net.net $samp $hid
		./scripts/test_mapreduce.sh dat/in/ 10 250 -u > results/$net.$samp.$hid.txt
	done
done

