#!/bin/bash -e
BNETS=`ls bnets | sed 's/.net//g'`
samples=1000000

for net in $BNETS
do
        python net_to_fg/read_net.py bnets/$net.net fg/$net.fg dat/$net.dat
        ./param_sharing fg/$net.fg $samples && mv fg/$net.tab tab/$net.tab &
done
