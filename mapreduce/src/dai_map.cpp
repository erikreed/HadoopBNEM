// erik reed
// erikreed@cmu.edu
#include "dai_mapreduce.h"
#include "boost/foreach.hpp"

Real EM_estep(MaximizationStep &mstep, const Evidence &evidence, InfAlg &inf) {
	Real likelihood = 0;

	inf.run();
	Real logZ = inf.logZ();

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

Real hadoop_iterate(vector<MaximizationStep>& msteps, const Evidence &e,
		InfAlg &inf) {
	Real likelihood = 0;
	for (size_t i = 0; i < msteps.size(); ++i)
		likelihood = EM_estep(msteps[i], e, inf);
	return likelihood;
}

string mapper(EMdata &dat) {
	FactorGraph fg = dat.fg;

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

	dat.emFile = ""; // reduce amount of serialization
	dat.tabFile = "";

	return emToString(dat);
}

string mapper(string &in) {
	EMdata dat = stringToEM(in);
	DAI_ASSERT(dat.ALEM_layer >= 0);

	if (dat.isConverged())
		return in;
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
	string datFile = "in/dat";
	string tabFile = ss.str();

	ifstream fin(datFile.c_str());
	assert(fin.good());

	using namespace boost::iostreams;
	filtering_streambuf<input> gzIn;
	gzIn.push(gzip_decompressor());
	gzIn.push(fin);
	boost::archive::binary_iarchive ia(gzIn);

	int objectsRead = 0;

	EMdata datForMapper;

	try {
	  while(true) {
	    ia >> datForMapper;

	    objectsRead++;

	    datForMapper.lastLikelihood = datForMapper.likelihood;
	    datForMapper.likelihood = 0;
	    datForMapper.emFile = emFile;
	    datForMapper.tabFile = tabFile;

	    string out = mapper(datForMapper);

	    // : is delimiter for Hadoop K-V; e.g. key*value
	    cout << datForMapper.bnID << ':' << datForMapper.bnID << '*';
	    // print data for reducer
	    cout << out << endl;
	  }
	} catch(boost::archive::archive_exception const& e) { }

	fin.close();
	assert(objectsRead > 0);

	return 0;
}
