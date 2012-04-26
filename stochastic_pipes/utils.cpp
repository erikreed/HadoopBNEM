// erik reed
// erikreed@cmu.edu

#include <dai/alldai.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <fstream>
#include <math.h>
#include <stdio.h>

using namespace std;
using namespace dai;




void randomize_fg(FactorGraph* fg) {
	vector<Factor> factors = fg->factors();
	size_t size = factors.size();
	for (size_t i = 0; i < size; i++) {
		Factor f = fg->factor(i);
		f.randomize();
		fg->setFactor(i, f, false);
	}
}



int main(int argc, char* argv[]) {

	if (argc < 3)
		throw;
	rnd_seed(time(NULL));
	FactorGraph fg;
	fg.ReadFromFile(argv[1]);


	for (size_t i=2; i<argc; i++) {
		randomize_fg(&fg);
		ofstream fout;
		fout.open (argv[i]);
		fout << fg << endl;
		fout.close();
	}

	return 0;
}
