#!/bin/bash -e
if [ $# -eq 0 ]; then                                                                         
    echo Usage: $0 txt_files [...]                                           
    exit 1                                                                                    
fi

for i in "$@"
do
	echo Number of iterations: $i
	egrep -o '*iters: [0-9]{1,}' $i | cut -d: -f2 | \
		awk '{ sum += $1; sumsq += $1*$1 } END { printf "Mean: %f, Std: %f\n", sum/NR, sqrt(sumsq/NR - (sum/NR)^2) }'

	echo

	echo Likelihood: $i
	egrep -o 'likelihood: -[0-9]*.[0-9]* iters:' $i | cut -d' ' -f2 | \
		awk '{ sum += $1; sumsq += $1*$1 } END { printf "Mean: %f, Std: %f\n", sum/NR, sqrt(sumsq/NR - (sum/NR)^2) }'
	echo
done
