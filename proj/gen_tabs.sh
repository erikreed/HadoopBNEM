BNETS="ADAPT_PHM09_T1 ADAPT_PHM09_T2 ADAPT_PHM10_P1 ADAPT_PHM10_P2"
samples=5000

for net in $BNETS
do
	python net_to_fg/read_net.py bnets/$net.net fg/$net.fg dat/$net.dat
	./simple/simple fg/$net.fg $samples && mv fg/$net.tab tab/$net.tab &
done
