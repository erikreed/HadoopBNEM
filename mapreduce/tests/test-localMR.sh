#!/bin/bash -e
# erik reed

net=asia
pop=10

SAMPLES=1000
HIDDEN=rand
max_iters=100

rm -rf dat/{in,out} in out
./scripts/init.sh dat/bnets/$net.net $SAMPLES $HIDDEN
echo $net: pop=$pop, max_iters=$max_iters


/usr/bin/time ./scripts/mr_local.sh dat/in $max_iters $pop -alem
#rm -rf dat/{in,out} in out

