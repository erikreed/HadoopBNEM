// erik reed
// erikreed@cmu.edu

#include "dai_mapreduce.h"
#include "boost/foreach.hpp"

// todo: refactor these
FactorGraph fg;
PropertySet infprops;

// generate these via something like ./scripts/init.sh dat/bnets/asia.net 100 3
// TODO: make these args
const char* EM_PATH = "dat/in/em";
const char* TAB_PATH = "dat/in/tab";
const char* FG_PATH = "dat/in/fg";

vector<EMAlg*> getAllRuns(vector<vector<EMAlg*> > &emAlgs) {
  vector<EMAlg*> ems;
  foreach(vector<EMAlg*> &layer, emAlgs) {
    foreach(EMAlg* em, layer) {
      ems.push_back(em);
    }
  }
  return ems;
}

// tons of memory leaks in here
EMAlg *initEMAlg(FactorGraph fg, PropertySet &infprops) {
  Evidence* e = new Evidence();
  randomize_fg(&fg);
  //    InfAlgPtr inf = InfAlgPtr(newInfAlg(INF_TYPE, fg, infprops));
  //    inf.get()->init();
  InfAlg* inf = newInfAlg(INF_TYPE, *fg.clone(), infprops);

  ifstream estream(TAB_PATH);
  e->addEvidenceTabFile(estream, *fg.clone());
  // Read EM specification
  ifstream emstream(EM_PATH);
  EMAlg *newEMalg = new EMAlg(*e, *inf, emstream);
  return newEMalg;
}

void checkRuns(vector<vector<EMAlg*> > &emAlgs, size_t* min_runs, size_t layer,
               size_t index) {
  assert(layer + 1 < emAlgs.size());

  EMAlg* em = emAlgs[layer][index];
  Real likelihood = em->logZ();

  if (emAlgs[layer + 1].size() < min_runs[layer + 1]) {
    emAlgs[layer + 1].push_back(em);
    if (verbose)
      cout << "Moved run from layer " << layer << " to " << layer + 1 << endl;
  } else {
    // find out if there is a worse run in next layer
    for (size_t j = 0; j < emAlgs[layer + 1].size(); j++) {
      Real next = emAlgs[layer + 1][j]->logZ();
      if (next < likelihood) {
        // discard em_next
        EMAlg* discarded = emAlgs[layer + 1][j];
        emAlgs[layer + 1].erase(emAlgs[layer + 1].begin() + j);
        delete discarded;
        if (verbose)
          cout << "Discarded run from layer " << layer + 1 << endl;
        // insert em
        emAlgs[layer + 1].push_back(em);
        if (verbose)
          cout << "Moved (better) run from layer " << layer << " to " << layer
              + 1 << endl;
        break;
      }
    }
  }

  // remove em from original layer
  emAlgs[layer].erase(emAlgs[layer].begin() + index);
  if (verbose)
    cout << "Deleted run from layer " << layer << endl;
}

void ALEM_check(vector<vector<EMAlg*> > &emAlgs, size_t* min_runs,
                size_t* ageLimit) {

  size_t runsTerminated = 0;
  // while non-terminated runs exist
  while (runsTerminated < pop_size) {

    // add EMs to first layer
    if (emAlgs[0].size() < min_runs[0]) {
      // insert k new EM runs
      //			int k = min_runs[0] - emAlgs[0].size();
      int k = min_runs[0] - getNumRuns(emAlgs);
      for (int j = 0; j < k; j++) {
        emAlgs[0].push_back(initEMAlg(fg, infprops));
        if (verbose)
          cout << "Adding run to layer 0. Total in layer 0: "
              << emAlgs[0].size() << endl;
      }
    }

    // iterate EMs over all layers
    vector<EMAlg*> ems = getAllRuns(emAlgs);
#pragma omp parallel for
    for (size_t i = 0; i < ems.size(); i++) {
      EMAlg* em = ems[i];
      if (!em->hasSatisfiedTermConditions()) {
        em->ALEM_active = true;
        em->iterate();
        if (verbose)
          cout << "iteration: layer " << i << " iterated to likelihood "
              << em->logZ() << " and iters=" << em->Iterations() << endl;
      }
    }

    // perform ALEM likelihood checking and move EMs
    // between layers
    for (size_t i = 0; i < numLayers; i++) {
      for (int j = emAlgs[i].size() - 1; j >= 0; j--) {
        EMAlg* em = emAlgs[i][j];
        // em trial has terminated
        if (em->hasSatisfiedTermConditions() && em->ALEM_active) {
          em->ALEM_active = false;
          runsTerminated++;
          if (verbose) {
            cout << "layer " << i << ", run " << j
                << " converged. runsTerminated=" << runsTerminated << endl;
          }
          // move run to completed EMs layer emAlgs[numLayers-1]
          EMAlg* converged = emAlgs[i][j];
          emAlgs[i].erase(emAlgs[i].begin() + j);
          emAlgs[numLayers - 1].push_back(converged);
        } else if (i < numLayers && em->Iterations() >= ageLimit[i]) {
          if (verbose)
            cout << "layer " << i << ", run " << j << " hit age limit of "
                << ageLimit[i] << endl;
          checkRuns(emAlgs, min_runs, i, j);
        }
      }
    }
  }
}

int main(int argc, char* argv[]) {
  rnd_seed(time(NULL));
  fg.ReadFromFile(FG_PATH);
  infprops = getProps();

  vector<vector<EMAlg*> > emAlgs;
  for (size_t i = 0; i < numLayers; i++)
    emAlgs.push_back(vector<EMAlg*> ());

  // beta_i (age limit of layer i)
  size_t* ageLimit = new size_t[numLayers];
  for (size_t i = 0; i < numLayers - 1; i++)
    ageLimit[i] = agegap * pow(2, i);
  ageLimit[numLayers - 1] = EM_MAX_ITER;

  // M_i (min number of runs of layer i)
  size_t* min_runs = new size_t[numLayers];
  min_runs[0] = min_runs_layer0;
  for (size_t i = 1; i < numLayers - 1; i++) {
    min_runs[i] = min_runs_intermediate;
  }
  min_runs[numLayers - 1] = pop_size;

  // initialize first layer
  for (size_t i = 0; i < min_runs[0]; i++) {

    EMAlg *newEMalg = initEMAlg(fg, infprops);
    //		newEMalg->iterate();

    // add EMAlg to first layer
    emAlgs[0].push_back(newEMalg);
    if (verbose)
      cout << "Adding run to layer 0. Total in layer 0: " << emAlgs[0].size()
          << endl;
  }

  // start ALEM
  ALEM_check(emAlgs, min_runs, ageLimit);

  // collect best likelihood
  Real bestLikelihood = -1e100;
  size_t bestIters = -1;
  cout << "ALEM complete. Finding best likelihood..." << endl;
  for (size_t i = 0; i < numLayers; i++) { //  last layer = converged runs
    for (size_t j = 0; j < emAlgs[i].size(); j++) {
      EMAlg* em = emAlgs[i][j];
      // only print out the inactive/converged EMs
      if (em->hasSatisfiedTermConditions())
        cout << "converged: layer " << i << ", run " << j << " likelihood: "
            << em->logZ() << " iters: " << em->Iterations() << endl;
      if (em->logZ() > bestLikelihood) {
        bestLikelihood = em->logZ();
        bestIters = em->Iterations();
      }

      delete em;
    }
  }

  cout << "Best: " << bestLikelihood << " iters: " << bestIters << endl;

  delete[] ageLimit;
  delete[] min_runs;

  return 0;
}
