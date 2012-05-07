# num_iters, num_trials
trials=50

for i in $(seq 1 1 $trials)
do
`which time` -v ./utils -b 0 25 &> utils.log.$i
`which time` -v ./alem &> alem.log.$i
done
