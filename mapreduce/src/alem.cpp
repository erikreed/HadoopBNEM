// erik reed
// erikreed@cmu.edu

#include "dai_mapreduce.h"

const size_t pop_size = 10; // i.e. EMruns, denoted N
const size_t numLayers = 6;
const double agegap = 5; // denoted a

int main(int argc, char* argv[]) {

	FactorGraph fg;
	fg.ReadFromFile("dat/fg");

	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set("verbose", (size_t) 0);
	infprops.set("updates", string("HUGIN"));
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("MAX_ITERS", EM_MAX_ITER);


	vector<InfAlg*> algs;
	vector<vector <EMAlg*> > emAlgs;
	for (size_t i = 1; i < numLayers; i++)
		emAlgs.push_back(vector<EMAlg*>());

	// beta_i (age limit of layer i)
	int* ageLimit = new int[numLayers];
	for (size_t i = 0; i < numLayers - 1; i++)
		ageLimit[i] = agegap * pow(2,i-1);
	ageLimit[numLayers-1] = EM_MAX_ITER;

	// M_i (min number of runs of layer i)
	int* min_runs = new int[numLayers];
	min_runs[0] = 5;
	for (size_t i = 1; i < numLayers - 1; i++)
		min_runs[i] = 2;
	min_runs[numLayers-1] = EM_MAX_ITER;


	// initialize InfAlgs
	for (size_t i = 0; i < pop_size; i++) {
		randomize_fg(&fg);
		InfAlg* inf = newInfAlg(INF_TYPE, fg, infprops);
		inf->init();
		algs.push_back(inf);
	}

	// initialize EMAlgs
	for (size_t i = 0; i < pop_size; i++) {
		Evidence e;
		ifstream estream("dat/tab");
		e.addEvidenceTabFile(estream, fg);

		// Read EM specification
		ifstream emstream("dat/em");

		// add EMAlg to first layer
		emAlgs[0].push_back(new EMAlg(e, *algs[i], emstream));

	}


	// perform initial EM run:
	for (size_t i = 0; i < pop_size; i++)
		emAlgs[0][i]->iterate();

	// collect best likelihood
	Real bestLikelihood = -1e100;
	for (size_t i = 0; i < numLayers; i++) {
		for (size_t j=0; j<emAlgs[i].size(); j++) {
			bestLikelihood = max(bestLikelihood, emAlgs[i][j]->logZ());
			delete emAlgs[i][j];
		}
	}

	cout << "Best: " << bestLikelihood << endl;

	for (size_t i = 0; i < pop_size; i++)
		delete algs[i];
	delete[] ageLimit;
	delete[] min_runs;

	return 0;
}
