#!/bin/bash

# params -- NODES = nodes to hide
NUM_SAMPLES=500
NODES="Health_ish136 Closed_breaker_ey136_op Voltage_breaker_ey136_op Health_breaker_ey136_op"


if test "$1" == "" ; then
	echo Usage: ./master.sh net/my_cool_net.net
	exit 1
fi

mkdir -p em
mkdir -p fg
mkdir -p tab
mkdir -p dat
mkdir -p out

NET=$1
filename=$(basename $NET)
filename=${filename%%.net}
FG=$filename.fg
DAT=$filename.dat
TAB=$filename.tab
HIDDEN=$filename.hid
EM=$filename.em
FIXED=$filename.fixed

echo Reading $NET
echo Creating: $FG $DAT
python net_to_fg/read_net.py $NET fg/$FG dat/$DAT
echo
./simple/simple fg/$FG $NUM_SAMPLES
echo Moving fg/$TAB to tab/$TAB
echo
mv fg/$TAB tab/$TAB

python net_to_fg/summarize_net.py dat/$DAT

echo
echo Hiding nodes: $NODES
echo Writing .tab with hidden nodes to dat/$DAT
echo

python net_to_fg/hide_vars.py tab/$TAB dat/$DAT tab/$HIDDEN $NODES

echo
echo Creating EM file em/$EM
echo

python net_to_fg/write_em.py fg/$FG dat/$DAT em/$EM

echo
echo Running EM using all initialization types
echo

simple/simple -all fg/$FG tab/$HIDDEN em/$EM
simple/simple fg/$FG tab/$HIDDEN em/$EM
mv output.dat.default out/$DAT.default
mv output.dat.noise out/$DAT.noise
mv output.dat.random out/$DAT.random
mv output.dat.uniform out/$DAT.uniform

echo
echo Plotting results in out/$DAT.INIT_TYPE.png
echo

python net_to_fg/plot_output.py out/$DAT.default out/$filename.default.png \
	Default Initialization $NUM_SAMPLES Samples
python net_to_fg/plot_output.py out/$DAT.noise out/$filename.noise.png \
	Noisy Initialization $NUM_SAMPLES Samples
python net_to_fg/plot_output.py out/$DAT.uniform out/$filename.uniform.png \
	Uniform Initialization $NUM_SAMPLES Samples
python net_to_fg/plot_output.py out/$DAT.random out/$filename.random.png \
	Random Initialization - $NUM_SAMPLES Samples

