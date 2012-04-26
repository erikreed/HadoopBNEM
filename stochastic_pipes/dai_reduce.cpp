// erik reed
// erikreed@cmu.edu

#include "dai_mapreduce.h"

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

// TODO: key val will be BN_ID:E1,E2,E3 where E_N corresponds to evidence set N
EMdata em_reduce(vector<string>& in) {
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

	return dat;
}

EMdata em_reduce(vector<EMdata>& in) {
	// using first EMdata to store e-step counts and create fg
	EMdata dat = in[0];
	FactorGraph fg;
	istringstream fgStream(dat.fgFile);
	fgStream >> fg;

	// collect stats for each set of evidence
	for (size_t i=1; i<in.size(); i++) {
		EMdata next = in[i];
		DAI_ASSERT(dat.msteps.size() == next.msteps.size());
		DAI_ASSERT(dat.bnID == next.bnID);

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

	return dat;
}


int main(int argc, char* argv[]) {

	//read data from mappers
	ostringstream ss;

	// get evidence
	string s;
	while (std::getline(std::cin, s))
		ss << s << '\n';

	string input = ss.str();

//	cout << input << endl;
//	return 0;
	vector<string> data = str_split(input, '\n');


	ifstream fin("in/pop");
	int pop_size;
	fin >> pop_size;
	fin.close();

	vector<EMdata>* datsForReducer = new vector<EMdata>[pop_size];


	for (size_t i=0; i<data.size(); i++) {
		string line = data[i];
		str_char_replace(line,'^','\n');
		vector<string> parts = str_split(line, '*');
//		cerr << parts[0] << endl;
		assert(parts.size() == 2);

		EMdata dat = stringToEM(parts[1]); // value
		int id = atoi(parts[0].c_str()); // key
		assert(id == dat.bnID && id >= 0 && id < pop_size);
		datsForReducer[dat.bnID].push_back(dat);
	}

	for (int id=0; id < pop_size; id++) {
		EMdata out = em_reduce(datsForReducer[id]);
		string outstring = emToString(out);
		str_char_replace(outstring,'\n','^');
		cout << outstring << endl;
	}

	delete[] datsForReducer;

//	ofstream fout;
//	fout.open ("out/lhood");
//	fout << dat.likelihood << endl;
//	fout.close();
//	fout.open ("out/fg");
//	fout << dat.fgFile << endl;
//	fout.close();
//	fout.open ("out/iters");
//	fout << dat.iter << endl;
//	fout.close();


//	cout << emToString(out) << endl;



	return 0;
}
