// erik reed
// erikreed@cmu.edu
#include "dai_mapreduce.h"

// run vanilla EM
Real doEmIters(char* fgIn, char* tabIn, char* emIn, int numIters, size_t *numItersRet) {
	string fixedIn = emIn;

	FactorGraph fg;
	fg.ReadFromFile(fgIn);

	randomize_fg(&fg);
	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops = getProps();

	InfAlg* inf = newInfAlg(INF_TYPE, fg, infprops);
	inf->init();

	// Read sample from file
	Evidence e;
	ifstream estream(tabIn);
	e.addEvidenceTabFile(estream, fg);
//	cout << "Number of samples: " << e.nrSamples() << endl;

	// Read EM specification
	ifstream emstream(emIn);
	EMAlg em(e, *inf, emstream);

	Real likelihood;

	// Iterate EM until convergence
	if (numIters > 0) {
		for (int i=0; i<numIters; i++)
			likelihood = em.iterate();
	}
	else {
		while (!em.hasSatisfiedTermConditions())
			likelihood= em.iterate();
	}

	*numItersRet = em.Iterations();

	// Clean up
	delete inf;
	//	return ss.str();
	return likelihood;
}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		cout << "usage:" << argv[0] << endl;
		cout << "-u when piping serialized EMdata in" << endl;
		cout << "-b num_iters num_trials for benchmarking w/o MR" << endl;
		cout << "\t num_iters < 0 results in iterating to convergence" << endl;
		cout << "no flags and list of files to initialize random serialized EMdatas" << endl;
		return 0;
	}
	if (argc == 2) {
		string flag = argv[1];
		if (flag == "-u"){
			// get post mapreduce data -- update
			bool terminated = true;
			string s;
			while (std::getline(std::cin, s)) {
				str_char_replace(s,'^','\n');
				EMdata dat = stringToEM(s);

				ostringstream outname;
				outname << "out/dat." << dat.bnID;
				ofstream fout;
				fout.open (outname.str().c_str());
				fout << s << endl;
				fout.close();

				// check if all BNs have converged
				if (terminated)
					terminated = dat.isConverged();

				cout << "iter: " << dat.iter << "\t likelihood: " << dat.likelihood << endl;
			}
			ofstream fout("out/converged");
			if (terminated)
				fout << 1 << endl;
			else
				fout << 0 << endl;
			fout.close();
		}

		return 0;
	}

	// for benchmarking EM -- non mapreduce
	if (string(argv[1]) == "-b") {
		// expecting num iters
		int numIters = atoi(argv[2]);
		int numTrials = atoi(argv[3]);

		Real bestLikelihood = -1e100;

		#pragma omp parallel for
		for (int i=0; i<numTrials; i++) {
			size_t numItersRet;
			Real l = doEmIters("dat/fg","dat/tab","dat/em",numIters,&numItersRet);
			cout << "likelihood: " << l << " iters: " << numItersRet << endl;

			#pragma omp critical
			{
				bestLikelihood = max(bestLikelihood,l);
			}
		}
		cout << "best likelihood: " << bestLikelihood << endl;
		return 0;
	}

	rnd_seed(time(NULL));
	FactorGraph fg;
	fg.ReadFromFile(argv[1]);

	string emFile = readFile("in/em");

	for (int i=2; i<argc; i++) {
		randomize_fg(&fg);

		EMdata datForMapper;
		datForMapper.iter = 0;
//		datForMapper.emFile = emFile;
		datForMapper.likelihood = 0;
		datForMapper.bnID = -1;
		datForMapper.ALEM_layer = 0;

		ostringstream ss;
		ss << fg;
		datForMapper.fgFile = ss.str();

		ofstream fout;
		fout.open (argv[i]);
		fout << emToString(datForMapper) << endl;
		fout.close();
	}

	return 0;
}
