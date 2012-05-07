// erik reed
// erikreed@cmu.edu
#include "dai_mapreduce.h"
#include "boost/foreach.hpp"

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

void checkRuns(vector<vector<EMdata> > &emAlgs, size_t*  min_runs, size_t layer, size_t index) {
	EMdata &em = emAlgs[layer][index];
	Real likelihood = em.likelihood;
	if (emAlgs[layer+1].size() < min_runs[layer+1]) {
		em.ALEM_layer = layer+1;
		emAlgs[layer+1].push_back(em);

		if (verbose)
			cout << "Moved run from layer " << layer << " to " << layer + 1 << endl;
	}
	else {
		// find out if there is a worse run in next layer
		for (size_t j=0; j<emAlgs[layer+1].size(); j++) {
			Real next = emAlgs[layer+1][j].likelihood;
			if (next < likelihood) {
				// discard em_next
				emAlgs[layer+1].erase(emAlgs[layer+1].begin() + j);
				if (verbose)
					cout << "Discarded run from layer " << layer +1 << endl;
				// insert em
				em.ALEM_layer = layer+1;
				emAlgs[layer+1].push_back(em);
				if (verbose)
					cout << "Moved (better) run from layer " << layer << " to " << layer + 1 << endl;
				break;
			}
		}
	}

	// remove em from original layer
	emAlgs[layer].erase(emAlgs[layer].begin()+index);
	if (verbose)
		cout << "Deleted run from layer " << layer << endl;
}

void ALEM_check(vector<vector<EMdata> > &emAlgs, size_t* min_runs, size_t* ageLimit) {
	// perform ALEM likelihood checking and move EMs
	// between layers
	for (size_t i=0; i<numLayers; i++) {
		for (int j=emAlgs[i].size()-1; j>=0; j--){
			EMdata &em = emAlgs[i][j];
			// em trial has terminated
			if (em.isConverged()) {
				// move run to completed EMs layer emAlgs[numLayers-1]
				EMdata &converged = emAlgs[i][j];
				converged.ALEM_layer = numLayers-1;
				emAlgs[numLayers-1].push_back(converged);
				emAlgs[i].erase(emAlgs[i].begin() + j);

			}
			else if (i < numLayers && em.iter >= ageLimit[i]) {
				if (verbose)
					cout << "layer " << i << ", run " << j <<
					" hit age limit of " << ageLimit[i] << endl;
				checkRuns(emAlgs,min_runs, i, j);
			}
		}
	}

}

string getFirstFG(vector<vector<EMdata> > &emAlgs) {
	foreach(vector<EMdata> &layer, emAlgs) {
		foreach(EMdata &em, layer) {
			return em.fgFile;
		}
	}
	return "ERROR";
}

string randomize_fg_str(string &fgStr) {
	FactorGraph fg;
	stringstream fgStream(fgStr);
	fgStream.precision(20);
	fgStream >> fg;
	randomize_fg(&fg);
	fgStream.clear();
	fgStream.precision(20);
	fgStream << fg;
	return fgStream.str();
}


int getMaxID(vector<vector<EMdata> > &emAlgs) {
	int id_max = -1;
	foreach(vector<EMdata> &layer, emAlgs) {
		foreach(EMdata &em, layer) {
			id_max = max(id_max,em.bnID);
		}
	}
	return id_max;
}

void alem(vector<vector<EMdata> > &emAlgs) {
	// beta_i (age limit of layer i)
	size_t* ageLimit = new size_t[numLayers];
	for (size_t i = 0; i < numLayers - 1; i++)
		ageLimit[i] = agegap * pow(2, i);
	ageLimit[numLayers - 1] = EM_MAX_ITER;

	// M_i (min number of runs of layer i)
	size_t* min_runs = new size_t[numLayers];
	min_runs[0] = min_runs_layer0;
	for (size_t i = 1; i < numLayers - 1; i++)
		min_runs[i] = min_runs_intermediate;
	min_runs[numLayers - 1] = pop_size;

	ALEM_check(emAlgs, min_runs, ageLimit);

	// add EMs to first layer
	if (emAlgs[0].size() < min_runs[0]) {
		// insert k new EM runs
//		int k = min_runs[0] - emAlgs[0].size();
		int k = min_runs[0] - getNumRuns(emAlgs);
		if (k > 0) {
			int currentID = getMaxID(emAlgs) + 1;
			for (int j=0; j<k; j++) {
				EMdata newEM;
				newEM.fgFile = getFirstFG(emAlgs);
				newEM.fgFile = randomize_fg_str(newEM.fgFile);
				newEM.iter = 0;
				newEM.likelihood = 0;
				newEM.bnID = currentID++;
				newEM.ALEM_layer = 0;
				emAlgs[0].push_back(newEM);
				if (verbose)
					cout << "Adding run to layer 0. Total in layer 0: "
					<< emAlgs[0].size() << endl;
			}
		}
	}

	delete[] ageLimit;
	delete[] min_runs;
}

int main(int argc, char* argv[]) {
	rnd_seed(time(NULL));
	if (argc == 1) {
		cout << "usage:" << argv[0] << endl;
		cout << "-u when piping serialized EMdata in" << endl;
		cout << "-alem when piping serialized EMdata in using ALEM" << endl;
		cout << "-b num_iters num_trials for benchmarking w/o MR" << endl;
		cout << "\t num_iters < 0 results in iterating to convergence" << endl;
		cout << "no flags and list of files to initialize random serialized EMdatas" << endl;
		return 0;
	}
	if (argc == 2) {
		string flag = argv[1];
		bool terminated = true;
		if (flag == "-u"){
			// get post mapreduce data -- update

			size_t numConverged = 0;
			size_t total=0;
			string s;
			Real bestLikelihood = -1e100;
			size_t bestIters = -1;
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
				if (dat.isConverged())
					numConverged++;
				total++;
				cout << "iter: " << dat.iter << "\t likelihood: " << dat.likelihood << endl;

				if (dat.likelihood > bestLikelihood) {
					bestLikelihood = dat.likelihood;
					bestIters = dat.iter;
				}

			}
			cout << "Runs converged: " << numConverged << "/" << total << endl;
			cout << "Best EM likelihood so far: " << bestLikelihood << ", iters: " << bestIters << endl;
		}

		else if (flag == "-alem") {
			size_t numConverged = 0;
			string s;
			Real bestLikelihood = -1e100;
			size_t bestIters = -1;

			// initialize ALEM structures
			vector<vector<EMdata> > emAlgs;
			for (size_t i = 0; i < numLayers ; i++)
				emAlgs.push_back(vector<EMdata> ());

			while (std::getline(std::cin, s)) {
				str_char_replace(s,'^','\n');
				EMdata dat = stringToEM(s);
				int layer = dat.ALEM_layer;
				assert(layer >= 0 && (size_t) layer < numLayers);
				emAlgs[layer].push_back(dat);


				if (dat.likelihood > bestLikelihood) {
					bestLikelihood = dat.likelihood;
					bestIters = dat.iter;
				}

				cout << "ID: " << dat.bnID <<  "\t layer: "<< dat.ALEM_layer << "\t iter: " << dat.iter << "\t likelihood: " << dat.likelihood << endl;
				if (dat.isConverged())
					numConverged++;
			}

			if (numConverged >= pop_size) {
				cout << "Best EM likelihood: " << bestLikelihood << ", iters: " << bestIters << endl;
				cout << "ALEM converged. Completed runs = " << numConverged << endl;
				terminated = true;
			}
			else {
				cout << "Best EM likelihood so far: " << bestLikelihood << ", iters: " << bestIters << endl;
				cout << "ALEM progress: " << numConverged << "/" << pop_size << " converged." << endl;
				terminated = false;
				alem(emAlgs);
			}



			foreach(vector<EMdata> &layer, emAlgs) {
				foreach(EMdata &em, layer) {
					ostringstream outname;
					outname << "out/dat." << em.bnID;
					ofstream fout;
					fout.open (outname.str().c_str());
					fout << emToString(em) << endl;
					fout.close();
				}
			}
		}

		ofstream fout("out/converged");
		if (terminated)
			fout << 1 << endl;
		else
			fout << 0 << endl;
		fout.close();

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

	FactorGraph fg;
	fg.ReadFromFile(argv[1]);

//	string emFile = readFile("in/em");

	for (int i=2; i<argc; i++) {
		randomize_fg(&fg);

		EMdata datForMapper;
		datForMapper.iter = 0;
//		datForMapper.emFile = emFile;
		datForMapper.likelihood = 0;
		datForMapper.bnID = -1;
		datForMapper.ALEM_layer = 0;

		ostringstream ss;
		ss.precision(20);
		ss << fg;
		datForMapper.fgFile = ss.str();

		ofstream fout;
		fout.open (argv[i]);
		fout << emToString(datForMapper) << endl;
		fout.close();
	}

	return 0;
}
