// erik reed
// erikreed@cmu.edu

#include <dai/alldai.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <fstream>
#include <math.h>
#include <stdio.h>
//#include "hadoop/Pipes.hh"
//#include "hadoop/TemplateFactory.hh"
//#include "hadoop/StringUtils.hh"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;
using namespace dai;

//#define DO_RANDOM_EM // does the many iterations of random EM
#define INTERMEDIATE_VALUES
#define NOISE_AMOUNT .05 // corresponding to 5%
#define INF_TYPE "JTREE"

// constants for compareEM(...)
const size_t EM_MAX_SAMPLES = 5000;
const size_t EM_SAMPLES_DELTA = 50;
const size_t EM_INIT_SAMPLES = 50;
const Real LIB_EM_TOLERANCE = 1e-9; // doesn't have an effect...
const size_t EM_MAX_ITER = 100;
//#define OLD_FIXING


struct EMdata {
	string emFile;
	string fgFile;
	string tabFile;
	size_t iter;
	Real likelihood;
	vector<MaximizationStep> msteps;
	int bnID;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & emFile;
		ar & fgFile;
		ar & tabFile;
		ar & iter;
		ar & likelihood;
		ar & msteps;
		ar & bnID;
	}
};





inline string emToString(EMdata &em) {
	ostringstream s;
	boost::archive::text_oarchive oa(s);
	oa << em;
	return s.str();
}

inline EMdata stringToEM(string &s) {
	istringstream ss(s);
	boost::archive::text_iarchive ia(ss);
	EMdata em;
	ia >> em;
	return em;
}

string convertInt(int number) {
	stringstream ss;//create a stringstream
	ss << number;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

string readFile(const char* path) {

	int length;
	char * buffer;

	ifstream is;
	is.open(path, ios::binary);

	// get length of file:
	is.seekg(0, ios::end);
	length = is.tellg();
	is.seekg(0, ios::beg);
	// allocate memory:
	buffer = new char[length];

	// read data as a block:
	is.read(buffer, length);
	is.close();
	string out = buffer;
	delete[] buffer;
	return out;
}

bool str_replace(std::string& str, const std::string& from,
		const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

void randomize_fg(FactorGraph* fg) {
	vector<Factor> factors = fg->factors();
	size_t size = factors.size();
	for (size_t i = 0; i < size; i++) {
		Factor f = fg->factor(i);
		f.randomize();
		fg->setFactor(i, f, false);
	}
}

vector<std::string> &str_split(const std::string &s, char delim, std::vector<
		std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
		elems.push_back(item);
	return elems;
}

std::vector<std::string> str_split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	return str_split(s, delim, elems);
}

void doEm(char* fgIn, char* tabIn, char* emIn, int init) {
	string fixedIn = emIn;

	FactorGraph fg;
	fg.ReadFromFile(fgIn);

	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set("verbose", (size_t) 1);
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
	cout << "Number of samples: " << e.nrSamples() << endl;

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
	infprops.set("verbose", (size_t) 1);
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
	cout << "Number of samples: " << e.nrSamples() << endl;

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
	}

	stringstream ss;
	ss << inf->fg() << endl;

	// Clean up
	delete inf;
	return ss.str();

}

string mapper(const string& in) {

	EMdata dat = stringToEM(in);

	FactorGraph fg;

	istringstream fgStream(fgIn);
	fgStream >> fg;

	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set("verbose", (size_t) 1);
	infprops.set("updates", string("HUGIN"));
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("LOG_Z_TOL_KEY", LIB_EM_TOLERANCE);
	infprops.set(EMAlg::LOG_Z_TOL_KEY, LIB_EM_TOLERANCE);

	InfAlg* inf = newInfAlg(INF_TYPE, fg, infprops);
	inf->init();

	// Read sample from file
	Evidence e;
	istringstream estream(tabIn);
	e.addEvidenceTabFile(estream, fg);
	cout << "Number of samples: " << e.nrSamples() << endl;

	// Read EM specification
	istringstream emstream(emIn);
	EMAlg em(e, *inf, emstream);

	// perform 1 iteration e-step
	Real likelihood = hadoop_iterate(em._msteps, em._evidence, em._estep);

	dat.iter++;
	dat.likelihood = likelihood;
	dat.msteps = em._msteps;

	// Clean up
	delete inf;

	return emToString(dat);
}

// key val will be BN_ID:E1,E2,E3 where E_N corresponds to evidence set N
string reduce(vector<string>& key) {
//
//	for (size_t i = 0; i < em._msteps.size(); i++)
//			em._msteps[i].maximize(em._estep.fg());
	return NULL;
}

int main(int argc, char* argv[]) {

	bool tests = false;
	size_t numMappers = 5;

	string emFile = readFile("dat/em");
	string fgFile = readFile("dat/fg");
	string tabFile = readFile("dat/tab");

	if (tests) {
		string s1 = testEM("dat/fg", "dat/tab", "dat/em", false);
		string s2 = testEM("dat/fg", "dat/tab", "dat/em", true);
		if (s1 != s2)
			throw;
	}

	vector<string> tabLines = str_split(tabFile, '\n');
	//	numMappers = tabLines.size()/numMappers > numMappers ? numMappers : tabLines.size()/numMappers;

	size_t samplesPerMapper = tabLines.size() / numMappers;


	size_t t = 0;
	for (size_t i = 0; i < numMappers; i++) {
		EMdata datForMapper;
		datForMapper.iter = 0;
		datForMapper.likelihood = 0;
		datForMapper.emFile = emFile;
		datForMapper.fgFile = fgFile;

		ostringstream evidence;
		for (size_t j = 0; j < samplesPerMapper && t < tabLines.size(); j++) {
			evidence << tabLines[t++] << '\n';
		}
		datForMapper.emFile = evidence.str();

		mapper(emToString(datForMapper));
	}

	return 0;
}

