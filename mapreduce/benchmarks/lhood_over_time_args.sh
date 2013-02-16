#!/bin/bash -e
# erik reed

if [ $# -ne 2 ]; then
  echo Usage: $0 bnet '-{sem,mem,alem}' 
  exit 1
fi

net=$1

if [ $2 == '-sem' ]; then
  pop=1
  ext=sem
  flag=-u
elif [ $2 == '-mem' ]; then
  pop=10
  ext=mem
  flag=-u
elif [ $2 == '-alem' ]; then
  pop=10
  ext=alem
  flag=-alem
else
  echo error! wrong type
  exit 1
fi

trials=1000
SAMPLES=1000
HIDDEN=rand
max_iters=100000
dir=results_lhood


export time_duration=600

mkdir -p $dir

echo $net: pop=$pop, max_iters=$max_iters

for trial in `seq 1 1 $trials`; do
  name=$dir/$net.$trial.$ext
  rm -f $name
  
  rm -rf dat/{in,out} in out
  ./scripts/init.sh dat/bnets/$net.net $SAMPLES $HIDDEN
  
  export start_time=`date +%s`
  while true; do
    /usr/bin/time ./scripts/mr_local.sh dat/in $max_iters $pop $flag | \
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

