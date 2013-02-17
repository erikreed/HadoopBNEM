#!/bin/bash -e
#erik reed
if [ $# -ne 3 ]; then
    echo Usage: $0 in.tab out.tab num_hidden
    exit 1
fi

in=$1
out=$2
n=$3
nodes=`head -1 $in | grep -o $'\t' | wc -l`

shopt -s nocasematch
if [[ $n == "half" ]]; then
	n=$((($nodes + 1) / 2))
	echo Number of hidden nodes set to half of total: $n
elif [[ $n == "rand" ]]; then
	n=$(($RANDOM % $nodes + 1))
	echo Number of hidden nodes set to random: $n
fi

echo Hiding $n of $(($nodes + 1)) nodes...

nodes=$(($nodes - $n))

hid=`shuf -i 1-$(($nodes + 1)) -n $n`
cmd="cut -f"
for c in $hid; do
	cmd+="$c,"
done
cmd="${cmd%?}"

cat $in | $cmd > $out

