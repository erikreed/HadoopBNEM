#!/bin/bash -e
# erik reed

mkdir -p results
trials=1
pop=10
mappers=16

net=water
for i in `seq 1 1 $trials`; do
	for samp in 40 80 120 160 200; do
		for hid in 15; do 	
			# 15 of 32 nodes hidden

			echo $net: samples=$samp, hidden=$hid, $i of $trials

			rm -rf dat/{in,out} in out
			./scripts/init.sh dat/bnets/$net.net $samp $hid
			/usr/bin/time -o results/$net.$samp.$hid.$i.log \
				./scripts/mr_streaming.sh dat/in -u $mappers $pop 2>&1 | \
				tee results/$net.$samp.$hid.$i.txt
			mv dat/out/log.txt results/$net.info

			/usr/bin/time -o results/$net.$samp.$hid.$i.alem.log \
				./scripts/mr_streaming.sh dat/in -alem $mappers $pop 2>&1 | \
				tee results/$net.$samp.$hid.$i.alem.txt
			mv dat/out/log.txt results/$net.alem.info
		done
	done
done

