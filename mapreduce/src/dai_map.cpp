// erik reed
// erikreed@cmu.edu
#include "dai_mapreduce.h"

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

	return likelihood;
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

string mapper(EMdata &dat) {
	FactorGraph fg;
	istringstream fgStream(dat.fgFile);
	fgStream >> fg;


	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops = getProps();

	InfAlg* inf = newInfAlg(INF_TYPE, fg, infprops);
	inf->init();

	// Read sample from file
	Evidence e;
	istringstream estream(dat.tabFile);
	e.addEvidenceTabFile(estream, fg);

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

int main(int argc, char* argv[]) {

	//read evidence header
	ostringstream ss;

	ss << readFile("in/tab_header");

	// get evidence
	string s;
	while (std::getline(std::cin, s))
		ss << s << '\n';


	string emFile = readFile("in/em");

	string tabFile = ss.str();

	ifstream fin("in/pop");
	int pop_size;
	fin >> pop_size;
	fin.close();

	for (int id=0; id< pop_size; id++) {
		// read fg corresponding to current BN ID
		ostringstream datName;
		datName << "in/dat." << id;
		string datFile = readFile(datName.str().c_str());

		EMdata datForMapper = stringToEM(datFile);
		datForMapper.lastLikelihood = datForMapper.likelihood;
		datForMapper.likelihood = 0;
		datForMapper.emFile = emFile;
		datForMapper.tabFile = tabFile;
		datForMapper.bnID = id;

		string out = mapper(datForMapper);

		//
		// * is delimiter for K-V; e.g. key*value
		// ^ is replacement for \n

		str_char_replace(out,'\n','^');

		// print BN_ID
		cout << id << '*';
		// print data for reducer
		cout << out << endl;
	}

	return 0;
}
