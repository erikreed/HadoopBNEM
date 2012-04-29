// erik reed
// erikreed@cmu.edu
#include "dai_mapreduce.h"

void doEmIters(char* fgIn, char* tabIn, char* emIn, int numIters) {
	string fixedIn = emIn;

	FactorGraph fg;
	fg.ReadFromFile(fgIn);

	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set("verbose", (size_t) 1);
	infprops.set("updates", string("HUGIN"));
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("MAX_ITERS", EM_MAX_ITER);

	InfAlg* inf = newInfAlg(INF_TYPE, fg, infprops);
	inf->init();

	// Read sample from file
	Evidence e;
	ifstream estream(tabIn);
	e.addEvidenceTabFile(estream, fg);
	cout << "Number of samples: " << e.nrSamples() << endl;

	// Read EM specification
	ifstream emstream(emIn);
	EMAlg em(e, *inf, emstream);

	// Iterate EM until convergence
	for (int i=0; i<numIters; i++)
		em.iterate();


	// Clean up
	delete inf;
	//	return ss.str();

}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		cout << "usage:" << endl;
		cout << "-u when piping serialized EMdata in" << endl;
		cout << "-b num_iters fg_file tab_file em_file for benchmarking w/o MR" << endl;
		cout << "no flags and list of files to initialize random serialized EMdatas" << endl;
		return 0;
	}
	if (argc == 2) {
		string flag = argv[1];
		if (flag == "-u"){
			// get post mapreduce data -- update
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

//				outname.clear();
//				outname << "in/fg." << dat.bnID;
//				fout.open (outname.str().c_str());
//				fout << dat.fgFile << endl;
//				fout.close();

				cout << "iter: " << dat.iter << "\t likelihood: " << dat.likelihood << endl;
			}
		}
		// create initial serialized files with iter=0

		return 0;
	}

	// for benchmarking EM -- non mapreduce
	if (string(argv[1]) == "-b") {
		// expecting num iters
		int numIters = atoi(argv[2]);
		int numTrials = atoi(argv[3]);
		for (int i=0; i<numTrials; i++)
			doEmIters("dat/fg","dat/tab","dat/em",numIters);
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
