mkdir -p em
mkdir -p fg
mkdir -p tab
mkdir -p dat

NET=$1
filename=$(basename $NET)
FG=${filename%%.net}.fg
DAT=${filename%%.net}.dat

echo Creating: $FG $DAT

python net_to_fg/read_net.py $NET fg/$FG dat/$DAT
