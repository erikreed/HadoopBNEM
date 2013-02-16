#!/bin/bash -e
# erik reed

net=asia
pop=1

SAMPLES=1000
HIDDEN=rand
max_iters=100000

export start_time=`date +%s`
export time_duration=5

rm -rf dat/{in,out} in out
./scripts/init.sh dat/bnets/$net.net $SAMPLES $HIDDEN
echo $net: pop=$pop, max_iters=$max_iters

while true; do
  /usr/bin/time ./scripts/mr_local.sh dat/in $max_iters $pop -u
  rm -rf dat/{in,out} in out
  
  if [ -n "$start_time" -a -n "$time_duration" ]; then
    dur=$((`date +%s` - $start_time))
    if [ $dur -ge $time_duration ]; then
      echo $time_duration seconds reached! Quitting...
      break
    fi
  fi
done

