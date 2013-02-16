#!/bin/bash -e
# erik reed

net=asia
pop=1

trials=1000
SAMPLES=1000
HIDDEN=rand
max_iters=100000
dir=results_lhood


export time_duration=5

mkdir -p $dir

echo $net: pop=$pop, max_iters=$max_iters

for trial in `seq 1 1 $trials`; do
  name=$dir/$net.$pop.$trial.sem
  rm -f $name
  
  rm -rf dat/{in,out} in out
  ./scripts/init.sh dat/bnets/$net.net $SAMPLES $HIDDEN
  
  export start_time=`date +%s`
  while true; do
    /usr/bin/time ./scripts/mr_local.sh dat/in $max_iters $pop -u | \
        tee -a $name
    
    if [ -n "$start_time" -a -n "$time_duration" ]; then
      dur=$((`date +%s` - $start_time))
      if [ $dur -ge $time_duration ]; then
        echo $time_duration seconds reached! Quitting...
        break
      fi
    fi
  done
done

