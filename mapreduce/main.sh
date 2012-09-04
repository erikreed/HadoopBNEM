#!/bin/bash
# erik reed

mkdir -p results

trials=10
iters=15
pop=10

# water tests -- 32 nodes
# single core benchmarks: ~20 seconds per iteration w/ pop 1
# for 10 samples
# -- so, estimated 80 hours for 1000 samples, 15 iters, pop=10
net=water
for i in `seq 1 1 $trials`; do
	for samp in 1000; do
		for hid in 15; do
			echo $net: samples=$samp, hidden=$hid, $i of $trials
			rm -rf dat/{in,out}
			./scripts/init.sh dat/bnets/$net.net $samp $hid
			./scripts/test_mapreduce.sh dat/in/ $iters $pop -u > results/$net.$samp.$hid.$i.txt
			#./scripts/test_mapreduce.sh dat/in/ $iters $pop -alem > results/$net.$samp.$hid.$i.alem.txt
		done
	done
done
#7za a $net.results.7z results
#rm -rf results
echo Done!
exit 0

#################################################
#################################################

# asia tests -- 8 nodes, 2hrs for ~50 trials
mkdir -p results
trials=1000                                                                                  
iters=15                                                                                     
pop=10

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
