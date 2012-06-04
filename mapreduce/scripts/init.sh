#!/bin/bash -e
# erik reed

if [ $# -ne 3 ]; then
    echo Usage: $0 bn.net num_samples num_hidden
    exit 1
fi

NUM_SAMPLES=$2
NUM_HIDDEN=$3

echo "-- Given parameters --"
echo "NumSamples: $NUM_SAMPLES" 
echo "Hidden nodes: $NUM_HIDDEN" 

rm -rf in
mkdir -p dat/em dat/fg dat/tab dat/dat in

NET=$1
filename=$(basename $NET)
filename=${filename%%.net}
FG=$filename.fg
DAT=$filename.dat
TAB=$filename.tab
HIDDEN=$filename.hid
EM=$filename.em
FIXED=$filename.fixed
INFO=$filename.info

echo Reading $NET
echo Creating: $FG $DAT
python scripts/read_net.py $NET dat/fg/$FG dat/dat/$DAT
echo

## generate tab file
# only gen tab file if it doesn't exist...
if [ -f dat/tab/$TAB ]
then
	echo dat/tab/$TAB already exists. Skipping tab creation.
	echo
else
	echo ERROR1
	exit 1
	#./simple/simple fg/$FG $NUM_SAMPLES
	#echo Moving fg/$TAB to tab/$TAB
	#echo
	#mv dat/fg/$TAB tab/$TAB
fi

echo
echo Hiding nodes: $NUM_HIDDEN
echo Writing .tab with hidden nodes to dat/tab/$samp
echo

./scripts/hide_nodes.sh dat/tab/$TAB dat/tab/$HIDDEN $NUM_HIDDEN

echo
echo Creating EM file em/$EM
echo
python scripts/write_em2.py dat/fg/$FG dat/dat/$DAT dat/em/$EM

rm dat/*.info
echo Creating dat/$INFO
echo "Created by $0 on `date`" > dat/$INFO
echo >> dat/$INFO
echo "Factorgraph: $filename" >> dat/$INFO
echo "NumSamples: $NUM_SAMPLES" >> dat/$INFO
echo "Hidden nodes: $NUM_HIDDEN" >> dat/$INFO
echo "CPTs [total_entries/num_parents]: \
	`python scripts/CPT_dim.py dat/dat/$DAT $HIDDEN_NODES`" >> dat/$INFO
echo >> dat/$INFO

rm -rf dat/in
mkdir -p dat/in
cp dat/fg/$FG dat/in/fg
cp dat/em/$EM dat/in/em

mv dat/tab/$HIDDEN dat/in/tmp
head -$(($NUM_SAMPLES + 2)) dat/in/tmp > dat/in/tab
rm dat/in/tmp

echo $0 successful.
