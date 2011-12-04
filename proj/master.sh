#!/bin/bash

# params -- NODES = nodes to hide
NUM_SAMPLES=500

# all nodes : Health_ish136 Closed_breaker_ey136_op Voltage_breaker_ey136_op Health_breaker_ey136_op Sensor_ish136 Command_breaker_ey136_op

#typical fixed: Health_ish136 Health_breaker_ey136_op

# mini1
HIDDEN_NODES=""
#HIDDEN_NODES="Closed_breaker_ey136_op Voltage_breaker_ey136_op Health_ish136 Health_breaker_ey136_op"
# mini2 TODO: fix
#HIDDEN_NODES="$HIDDEN_NODES Closed_breaker_ey236_op Voltage_breaker_ey236_op Sensor_ish236 Command_breaker_ey236_op"
# mini3 TODO: fix
#HIDDEN_NODES="$HIDDEN_NODES Closed_breaker_ey336_op Voltage_breaker_ey336_op Sensor_ish336 Command_breaker_ey336_op"

# default -- just comment/uncomment remaining
FIXED_NODES=""
# mini1
#FIXED_NODES="-F Health_ish136 -F Health_breaker_ey136_op" # prefix each with -F
# mini2
#FIXED_NODES="$FIXED_NODES -F Health_ish236 -F Health_breaker_ey236_op"
# mini3
#FIXED_NODES="$FIXED_NODES -F Health_ish336 -F Health_breaker_ey336_op"

SHARED_NODES="-S health breaker" # prefix each with -S

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
SAMPLES=$filename.samp
INFO=$filename.info

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

python net_to_fg/hide_vars.py tab/$TAB dat/$DAT tab/$HIDDEN $HIDDEN_NODES

echo
echo Creating EM file em/$EM
echo

python net_to_fg/write_em.py fg/$FG dat/$DAT em/$EM $SHARED_NODES $FIXED_NODES

echo
echo Running EM using all initialization types
echo

simple/simple -all fg/$FG tab/$HIDDEN em/$EM
mv output.dat.default out/$DAT.default
mv output.dat.noise out/$DAT.noise
mv output.dat.random out/$DAT.random
mv output.dat.uniform out/$DAT.uniform

echo Creating out/$INFO
echo "Created by master.sh on `date`" > out/$INFO
echo >> out/$INFO
echo "Factorgraph: $filename" >> out/$INFO
echo "NumSamples: $NUM_SAMPLES" >> out/$INFO
echo "Hidden nodes: $HIDDEN_NODES" >> out/$INFO
echo "Fixed nodes: $FIXED_NODES" >> out/$INFO
echo "Shared nodes: $SHARED_NODES" >> out/$INFO

#TODO: currently, the .tab file is recreated not using hidden nodes. integrate hide_vars.py
#echo
#echo Running EM with increasing number of samples for all initialization types
#echo
#simple/simple -s -all fg/$FG em/$EM
#mv output.samp.default out/$SAMPLES.default
#mv output.samp.noise out/$SAMPLES.noise
#mv output.samp.random out/$SAMPLES.random
#mv output.samp.uniform out/$SAMPLES.uniform
#rm fg/*.tab
rm em/*.fixed

echo
echo Plotting results in out/$DAT.INIT_TYPE.png
echo

python net_to_fg/plot_output.py out/$DAT.default out/$filename.default.png \
	"Default Initialization - $NUM_SAMPLES Samples"
python net_to_fg/plot_output.py out/$DAT.noise out/$filename.noise.png \
	"Noisy Initialization - $NUM_SAMPLES Samples"
python net_to_fg/plot_output.py out/$DAT.uniform out/$filename.uniform.png \
	"Uniform Initialization - $NUM_SAMPLES Samples"
python net_to_fg/plot_output.py out/$DAT.random out/$filename.random.png \
	"Random Initialization - $NUM_SAMPLES Samples"


