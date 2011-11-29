NUM_SAMPLES=500

mkdir -p em
mkdir -p fg
mkdir -p tab
mkdir -p dat

NET=$1
filename=$(basename $NET)
FG=${filename%%.net}.fg
DAT=${filename%%.net}.dat
TAB=${filename%%.net}.tab

echo Reading $NET
echo Creating: $FG $DAT
python net_to_fg/read_net.py $NET fg/$FG dat/$DAT

./simple/simple fg/$FG $NUM_SAMPLES
echo Moving fg/$TAB to tab/$TAB
mv fg/$TAB tab/$TAB
