// erik reed
// erikreed@cmu.edu

#include "dai_mapreduce.h"

// todo: refactor these
FactorGraph fg;
PropertySet infprops;

// tons of memory leaks in here
EMAlg *initEMAlg(FactorGraph fg, PropertySet &infprops) {
    Evidence* e = new Evidence();
    randomize_fg(&fg);
//    InfAlgPtr inf = InfAlgPtr(newInfAlg(INF_TYPE, fg, infprops));
//    inf.get()->init();
    InfAlg* inf = newInfAlg(INF_TYPE, *fg.clone(), infprops);

    ifstream estream("dat/tab");
    e->addEvidenceTabFile(estream, *fg.clone());
    // Read EM specification
    ifstream emstream("dat/em");
    EMAlg *newEMalg = new EMAlg(*e, *inf, emstream);
    return newEMalg;
}

void checkRuns(vector<vector<EMAlg*> > &emAlgs, size_t*  min_runs, size_t layer, size_t index) {
	assert(layer+1 < emAlgs.size());

	EMAlg* em = emAlgs[layer][index];
	Real likelihood = em->logZ();

	if (emAlgs[layer+1].size() < min_runs[layer+1]) {
		emAlgs[layer+1].push_back(em);
		if (verbose)
			cout << "Moved run from layer " << layer << " to " << layer + 1 << endl;
	}
	else {
		// find out if there is a worse run in next layer
		for (size_t j=0; j<emAlgs[layer+1].size(); j++) {
			Real next = emAlgs[layer+1][j]->logZ();
			if (next < likelihood) {
				// discard em_next
				EMAlg* discarded = emAlgs[layer+1][j];
				emAlgs[layer+1].erase(emAlgs[layer+1].begin() + j);
				delete discarded;
				cout << "Deleted run from layer " << layer +1 << endl;
				// insert em
				emAlgs[layer+1].push_back(em);
				if (verbose)
					cout << "Moved run from layer " << layer << " to " << layer + 1 << endl;
				break;
			}
		}
	}

	// remove em from original layer
	emAlgs[layer].erase(emAlgs[layer].begin()+index);
	if (verbose)
		cout << "Deleted run from layer " << layer << endl;
}

void ALEM_check(vector<vector<EMAlg*> > &emAlgs, size_t* min_runs, size_t* ageLimit) {

	size_t runsTerminated = 0;
	// while non-terminated runs exist
	while (runsTerminated < pop_size) {
		for (size_t i=0; i<numLayers; i++) {

			// if first layer...
			if (i == 0 &&  emAlgs[i].size() < min_runs[0]) {
				// insert k new EM runs
				int k = min_runs[i] - emAlgs[i].size();
				for (int j=0; j<k; j++) {
					emAlgs[i].push_back(initEMAlg(fg,infprops));
					if (verbose)
						cout << "Adding run to layer 0. Total in layer 0: "
						<< emAlgs[i].size() << endl;
				}
				// todo: verify that all of layer 0 is iterated
			}

			for (int j=emAlgs[i].size()-1; j>=0; j--){
				EMAlg* em = emAlgs[i][j];
				assert(em != NULL);

				if (!em->hasSatisfiedTermConditions()) {

					em->iterate();
					if (verbose)
						cout << "iteration: layer " << i << ", run " << j <<
						" iterated to likelihood " << em->logZ() << " and iters=" <<
						em->Iterations() << endl;
				}

				// em trial has terminated
				if (em->hasSatisfiedTermConditions()) {
					runsTerminated++;
					cout << "layer " << i << ", run " << j <<
							" converged. runsTerminated=" << runsTerminated << endl;
					// move run to comleted EMs layer emAlgs[numLayers]
					EMAlg* converged = emAlgs[i][j];
					emAlgs[i].erase(emAlgs[i].begin() + j);
					emAlgs[numLayers].push_back(converged);
				}
				else if (em->Iterations() >= ageLimit[i]) {
					if (verbose)
						cout << "layer " << i << ", run " << j <<
						" hit age limit of " << ageLimit[i] << endl;
					checkRuns(emAlgs,min_runs, i, j);
				}
			}
		}
	}
}



int main(int argc, char* argv[]) {

	fg.ReadFromFile("dat/fg");
	infprops.set("verbose", (size_t) 0);
	infprops.set("updates", string("HUGIN"));
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("MAX_ITERS", EM_MAX_ITER);

	vector<vector<EMAlg*> > emAlgs;
	for (size_t i = 0; i < numLayers + 1; i++)
		emAlgs.push_back(vector<EMAlg*> ());

	// beta_i (age limit of layer i)
	size_t* ageLimit = new size_t[numLayers];
	for (size_t i = 0; i < numLayers - 1; i++)
		ageLimit[i] = agegap * pow(2, i);
	ageLimit[numLayers - 1] = EM_MAX_ITER;

	// M_i (min number of runs of layer i)
	size_t* min_runs = new size_t[numLayers];
	min_runs[0] = 5;
	for (size_t i = 1; i < numLayers - 1; i++)
		min_runs[i] = 2;
	min_runs[numLayers - 1] = pop_size; // todo: verify


	// initialize first layer
	for (size_t i = 0; i < min_runs[0]; i++) {

		EMAlg *newEMalg = initEMAlg(fg, infprops);
//		newEMalg->iterate();

		// add EMAlg to first layer
		emAlgs[0].push_back(newEMalg);
		if (verbose)
			cout << "Adding run to layer 0. Total in layer 0: "
			<< emAlgs[0].size() << endl;
	}

	// start ALEM
	ALEM_check(emAlgs, min_runs, ageLimit);

	// collect best likelihood
	Real bestLikelihood = -1e100;
	if (verbose)
		cout << "ALEM complete. Finding best likelihood..." << endl;
	for (size_t i = 0; i < numLayers + 1; i++) { //  last layer = converged runs
		for (size_t j = 0; j < emAlgs[i].size(); j++) {
			if (verbose)
				cout << "iteration: layer " << i << ", run " << j <<
				" likelihood: " << emAlgs[i][j]->logZ() << endl;
			bestLikelihood = max(bestLikelihood, emAlgs[i][j]->logZ());
			delete emAlgs[i][j];
		}
	}

	cout << "Best: " << bestLikelihood << endl;

	delete[] ageLimit;
	delete[] min_runs;

	return 0;
}
