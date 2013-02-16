#!/bin/bash -e
# erik reed

pop=250
mappers=2
reducers=2
nets=asia

SAMPLES=1000
HIDDEN=rand

for net in $nets; do
	rm -rf dat/{in,out} in out
	./scripts/init.sh dat/bnets/$net.net $SAMPLES $HIDDEN
	echo $net: pop=$pop, mappers=$mappers

	/usr/bin/time ./scripts/mr_streaming.sh dat/in -u $mappers $reducers $pop
done
#rm -rf dat/{in,out} in out


