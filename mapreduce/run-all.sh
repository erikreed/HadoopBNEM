for bnet in `find dat/bnets/ -type f -exec basename {} .net \;`; do
  for i in sem mem alem; do
    echo ./benchmarks/lhood_over_time_args.sh $bnet -$i
  done
done

