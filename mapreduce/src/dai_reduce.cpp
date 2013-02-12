// erik reed
// erikreed@cmu.edu

#include "dai_mapreduce.h"
#include <map>

Real EM_estep(MaximizationStep &mstep, const Evidence &evidence, InfAlg &inf) {
  Real logZ = 0;
  Real likelihood = 0;

  inf.run();
  logZ = inf.logZ();

  // Expectation calculation
  for (Evidence::const_iterator e = evidence.begin(); e != evidence.end(); ++e) {
    InfAlg* clamped = inf.clone();
    // Apply evidence
    for (Evidence::Observation::const_iterator i = e->begin(); i != e->end(); ++i)
      clamped->clamp(clamped->fg().findVar(i->first), i->second);
    clamped->init();
    clamped->run();

    likelihood += clamped->logZ() - logZ;

    mstep.addExpectations(*clamped);

    delete clamped;
  }

  return likelihood;
}

bool emHasSatisfiedTermConditions(size_t iter, Real previous, Real current) {
  if (iter >= EM_MAX_ITER)
    return true;
  else if (iter < 3)
    // need at least 2 to calculate ratio
    // Also, throw away first iteration, as the parameters may not
    // have been normalized according to the estimation method
    return false;
  else {
    if (previous == 0)
      return false;
    Real diff = current - previous;
    if (diff < 0) {
      std::cerr << "Error: in EM log-likehood decreased from " << previous
          << " to " << current << std::endl;
      return true;
    }
    return (diff / fabs(previous)) <= LIB_EM_TOLERANCE;
  }
}

EMdata em_reduce(vector<EMdata>& in) {
  // using first EMdata to store e-step counts and create fg
  EMdata dat = in[0];
  if (dat.isConverged()) {
    dat.alemItersActive++;
    return dat;
  }
  FactorGraph fg = dat.fg;

  // collect stats for each set of evidence
  for (size_t i = 1; i < in.size(); i++) {
    EMdata next = in[i];
    DAI_ASSERT(dat.msteps.size() == next.msteps.size());
    DAI_ASSERT(dat.bnID == next.bnID && dat.ALEM_layer == next.ALEM_layer);

    for (size_t j = 0; j < dat.msteps.size(); j++)
      dat.msteps[j].addExpectations(next.msteps[j]);

    dat.likelihood += next.likelihood;
  }

  // m-step
  for (size_t i = 0; i < dat.msteps.size(); i++)
    dat.msteps[i].maximize(fg);

  // save new fg, clear counts, increment iter
  dat.fg = fg;
  dat.msteps.clear();
  dat.iter++;
  dat.alemItersActive = dat.iter;

  dat.emFile = "";  // reduce amount of serialization
  dat.tabFile = "";
  return dat;
}

int main(int argc, char* argv[]) {
  //read data from mappers

  set<int> ids;
  string line;
  vector<EMdata> dats;
  while (std::getline(std::cin, line)) {
    vector<string> parts = str_split(line, '*');
    assert(parts.size() == 2);

    EMdata dat = stringToEM(parts[1]); // value
    int id = atoi(parts[0].c_str()); // key
    assert(id == dat.bnID && id >= 0);

    if (dats.empty()) {
      // thrown if the key/value pairs aren't sorted properly
      assert(!ids.count(id));
      dats.push_back(dat);
    } else {
      int lastId = dats.back().bnID;
      if (lastId == id) {
        dats.push_back(dat);
      } else {
        // new BN, so reduce
        EMdata out = em_reduce(dats);
        string outstring = emToString(out);
        cout << outstring << endl;

        dats.clear();
        dats.push_back(dat);
      }
    }
    ids.insert(id);
  }

  if (!dats.empty()) {
    EMdata out = em_reduce(dats);
    string outstring = emToString(out);
    cout << outstring << endl;
  }

  if (ids.empty()) {
    cerr << "No data for reducer" << endl;
    return 0;
  }

  return 0;
}
