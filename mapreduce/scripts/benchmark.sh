# num_iters, num_trials
rm -f utils.log alem.log
trials=1

for i in $(seq 1 1 $trials)
do
`which time` -v ./utils -b 0 25 &>> utils.log
`which time` -v ./alem &>> alem.log
done
