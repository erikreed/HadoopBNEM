# num_iters, num_trials
for i in $(seq 1 1 15)
do
`which time` -v ./utils -b 0 10 >> utils.log
`which time` -v ./alem >> alem.log
done
