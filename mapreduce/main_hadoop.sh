#!/bin/bash -e
# erik reed

mkdir -p results
trials=5
pop=10
mappers=$pop

net=water
for i in `seq 1 1 $trials`; do
	for samp in 200 400 600 800 1000; do
		for hid in 15; do 	
			# 15 of 32 nodes hidden

			echo $net: samples=$samp, hidden=$hid, $i of $trials

			rm -rf dat/{in,out} in out
			./scripts/init.sh dat/bnets/$net.net $samp $hid
			time ./scripts/streaming.sh dat/in -u $mappers $pop | \
				tee results/$net.$samp.$hid.$i.txt
			mv dat/out/log.txt results/$net.info

			time ./scripts/streaming.sh dat/in -alem $mappers $pop | \
				tee results/$net.$samp.$hid.$i.alem.txt
			mv dat/out/log.txt results/$net.alem.info
		done
	done
done

