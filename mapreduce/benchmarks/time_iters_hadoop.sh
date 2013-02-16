#!/bin/bash -e
# erik reed

RESULTS=results_time
mkdir -p $RESULTS
pops="1 10 100 1000 10000"
mappers="5 10 15"
nets="asia alarm" #TODO: water

SAMPLES=1000
HIDDEN=rand

for net in $nets; do
	rm -rf dat/{in,out} in out
	./scripts/init.sh dat/bnets/$net.net $SAMPLES $HIDDEN
	for mapper in $mappers; do
		for pop in $pops; do 	
			echo $net: pop=$pop, mappers=$mapper
			if [ $pop -ge 1000 ]; then
        reducers=15
      else
        reducers=1
      fi
      /usr/bin/time -o $RESULTS/$net.$mapper.$pop.log \
          ./scripts/mr_streaming.sh dat/in -u $mapper $reducers $pop 2>&1 | \
          tee $RESULTS/$net.$mapper.$pop.txt
			mv dat/out/log.txt $RESULTS/$net.info
		done
	done
done
rm -rf dat/{in,out} in out

