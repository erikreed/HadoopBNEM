// erik reed
// erikreed@cmu.edu

#include <dai/alldai.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "include/hadoop/Pipes.hh"
#include "include/hadoop/TemplateFactory.hh"
#include "include/hadoop/StringUtils.hh"
#include "dai_mapreduce.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>


void doEm(char* fgIn, char* tabIn, char* emIn, int init) {
	string fixedIn = emIn;

	FactorGraph fg;
	fg.ReadFromFile(fgIn);

	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set("verbose", (size_t) 0);
	infprops.set("updates", string("HUGIN"));
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("LOG_Z_TOL_KEY", LIB_EM_TOLERANCE);
	infprops.set(EMAlg::LOG_Z_TOL_KEY, LIB_EM_TOLERANCE);

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

	// Iterate EM until convergence
	while (!em.hasSatisfiedTermConditions()) {
		em.iterate();
	}

	stringstream ss;
	ss << inf->fg() << endl;

	// Clean up
	delete inf;
	//	return ss.str();

}

Real EM_estep(MaximizationStep &mstep, const Evidence &evidence, InfAlg &inf) {
	Real logZ = 0;
	Real likelihood = 0;

	inf.run();
	logZ = inf.logZ();

	// Expectation calculation
	for (Evidence::const_iterator e = evidence.begin(); e != evidence.end(); ++e) {
		InfAlg* clamped = inf.clone();
		// Apply evidence
		for (Evidence::Observation::const_iterator i = e->begin(); i
				!= e->end(); ++i)
			clamped->clamp(clamped->fg().findVar(i->first), i->second);
		clamped->init();
		clamped->run();

		likelihood += clamped->logZ() - logZ;

		mstep.addExpectations(*clamped);

		delete clamped;
	}

	// Maximization of parameters
	//    mstep.maximize( _estep.fg() );

	return likelihood;
}

bool emHasSatisfiedTermConditions(size_t iter, Real previous, Real current) {
    if( iter >= EM_MAX_ITER )
        return true;
    else if( iter < 3 )
        // need at least 2 to calculate ratio
        // Also, throw away first iteration, as the parameters may not
        // have been normalized according to the estimation method
        return false;
    else {
        if( previous == 0 )
            return false;
        Real diff = current - previous;
        if( diff < 0 ) {
            std::cerr << "Error: in EM log-likehood decreased from " << previous << " to " << current << std::endl;
            return true;
        }
        return (diff / fabs(previous)) <= LIB_EM_TOLERANCE;
    }
}

//TODO: sum likelihood when evidence is split up
Real hadoop_iterate(vector<MaximizationStep>& msteps, const Evidence &e,
		InfAlg &inf) {
	Real likelihood = 0;
	for (size_t i = 0; i < msteps.size(); ++i)
		likelihood = EM_estep(msteps[i], e, inf);
	//    _lastLogZ.push_back( likelihood );
	//    ++_iters;
	return likelihood;
}

string testEM(char* fgIn, char* tabIn, char* emIn, bool serial) {

	FactorGraph fg;
	fg.ReadFromFile(fgIn);

	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set("verbose", (size_t) 0);
	infprops.set("updates", string("HUGIN"));
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("LOG_Z_TOL_KEY", LIB_EM_TOLERANCE);
	infprops.set(EMAlg::LOG_Z_TOL_KEY, LIB_EM_TOLERANCE);

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

	// Iterate EM until convergence
	while (!em.hasSatisfiedTermConditions()) {
		Real likelihood = hadoop_iterate(em._msteps, em._evidence, em._estep);
		em._iters++;
		em._lastLogZ.push_back(likelihood);

		if (serial) {
			ostringstream s;
			vector<MaximizationStep> m = em._msteps;

			boost::archive::text_oarchive oa(s);
			oa << m;

			istringstream s2(s.str());

			boost::archive::text_iarchive ia(s2);
			// read class state from archive
			vector<MaximizationStep> m2;
			ia >> m2;
			em._msteps = m2;

		}
		for (size_t i = 0; i < em._msteps.size(); i++)
			em._msteps[i].maximize(em._estep.fg());
		cout << em._iters << '\t' << likelihood << endl;
	}

	stringstream ss;
	ss << inf->fg() << endl;

	// Clean up
	delete inf;
	return ss.str();

}

string mapper(EMdata &dat) {
	FactorGraph fg;
	istringstream fgStream(dat.fgFile);
	fgStream >> fg;


	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set("verbose", (size_t) 0);
	infprops.set("updates", string("HUGIN"));
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("LOG_Z_TOL_KEY", LIB_EM_TOLERANCE);
	infprops.set(EMAlg::LOG_Z_TOL_KEY, LIB_EM_TOLERANCE);

	InfAlg* inf = newInfAlg(INF_TYPE, fg, infprops);
	inf->init();

	// Read sample from file
	Evidence e;
	istringstream estream(dat.tabFile);
	e.addEvidenceTabFile(estream, fg);
//	cout << "Number of samples: " << e.nrSamples() << endl;

	// Read EM specification
	istringstream emstream(dat.emFile);
	EMAlg em(e, *inf, emstream);

	// perform 1 iteration e-step
	Real likelihood = hadoop_iterate(em._msteps, em._evidence, em._estep);

	dat.likelihood = likelihood;
	dat.msteps = em._msteps;

	// Clean up
	delete inf;

	return emToString(dat);
}

string mapper(string &in) {
	EMdata dat = stringToEM(in);
	return mapper(dat);
}

// TODO: key val will be BN_ID:E1,E2,E3 where E_N corresponds to evidence set N
string em_reduce(vector<string>& in) {
	// using first EMdata to store e-step counts and create fg
	EMdata dat = stringToEM(in[0]);
	FactorGraph fg;
	istringstream fgStream(dat.fgFile);
	fgStream >> fg;

	// collect stats for each set of evidence
	for (size_t i=1; i<in.size(); i++) {
		EMdata next = stringToEM(in[i]);
		DAI_ASSERT(dat.msteps.size() == next.msteps.size());

		for (size_t j = 0; j < dat.msteps.size(); j++) {
			dat.msteps[j].addExpectations(next.msteps[j]);
			dat.likelihood += next.likelihood;
		}
	}

	// m-step
	for (size_t i=0; i<dat.msteps.size(); i++)
		dat.msteps[i].maximize(fg);

	ostringstream newFg;
	newFg << fg;

	// save new fg, clear counts, increment iter
	dat.fgFile = newFg.str();
	dat.msteps.clear();
	dat.iter++;

	return emToString(dat);
}


int main(int argc, char* argv[]) {

	//read evidence header
	ostringstream ss;

	ss << readFile("in/tab_header");

	// get evidence
	string s;
	while (std::getline(std::cin, s))
		ss << s << '\n';


	string emFile = readFile("in/em");
	string fgFile = readFile("in/fg");
	string tabFile = ss.str();

	EMdata datForMapper;
	datForMapper.iter = 0;
	datForMapper.likelihood = 0;
	datForMapper.emFile = emFile;
	datForMapper.fgFile = fgFile;
	datForMapper.tabFile = tabFile;
	datForMapper.bnID = '1'; //TODO

	string out = mapper(datForMapper);

	//
	// * is delimiter for K-V; e.g. key*value
	// ^ is replacement for \n

	str_char_replace(out,'\n','^');

	// print BN_ID
	cout << '1' << '*';
	// print data for reducer
	cout << out << endl;

	return 0;
}
