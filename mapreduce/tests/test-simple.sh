#!/bin/bash -e
# erik reed

pop=1
mappers=1
reducers=1
net=asia

SAMPLES=1
HIDDEN=rand

rm -rf dat/{in,out} in out
./scripts/init.sh dat/bnets/$net.net $SAMPLES $HIDDEN
echo $net: pop=$pop, mappers=$mappers

/usr/bin/time ./scripts/streaming.sh dat/in -u $mappers $reducers $pop
rm -rf dat/{in,out} in out


