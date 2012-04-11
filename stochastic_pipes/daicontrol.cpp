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

using namespace std;
using namespace dai;

//#define DO_RANDOM_EM // does the many iterations of random EM
#define INTERMEDIATE_VALUES
#define NOISE_AMOUNT .05 // corresponding to 5%
#define INF_TYPE "JTREE"

// constants for compareEM(...)
const size_t EM_MAX_SAMPLES =5000;
const size_t EM_SAMPLES_DELTA= 50;
const size_t EM_INIT_SAMPLES =50;
const Real LIB_EM_TOLERANCE = 1e-9; // doesn't have an effect...
const size_t EM_MAX_ITER = 100;
//#define OLD_FIXING


string generateTab(const char* input, int numSamples,
		const char* custom_out) {
	string outName = input;
	if (custom_out != NULL) {
		outName = custom_out;
	} else {
		string::size_type pos = 0;
		pos = outName.find(".fg", pos);
		if (pos != string::npos)
			outName.replace(pos, outName.size(), ".tab");
		else
			outName.append(".tab");
	}
	if (numSamples < 1)
		throw;
	FactorGraph factorIn;
	factorIn.ReadFromFile(input);
	cout << "Reading " << input << endl;

	// Output some information about the factorgraph
	cout << factorIn.nrVars() << " variables" << endl;
	cout << factorIn.nrFactors() << " factors" << endl;
	cout << "Generating " << outName << endl;
	cout << "Number of samples: " << numSamples << endl;

	srand(time(NULL)); // set random seed

	// Prepare a Gibbs sampler
	PropertySet gibbsProps;
	gibbsProps.set("maxiter", size_t(numSamples)); // number of Gibbs sampler iterations
	gibbsProps.set("burnin", size_t(0));
	gibbsProps.set("verbose", size_t(0));
	Gibbs gibbsSampler(factorIn, gibbsProps);

	// Open a .tab file for writing
	ofstream outfile;

	outfile.open(outName.c_str());
	if (!outfile.is_open())
		throw "Cannot write to file!";

	// Write header (consisting of variable labels)
	for (size_t i = 0; i < factorIn.nrVars(); i++)
		outfile << (i == 0 ? "" : "\t") << factorIn.var(i).label();
	outfile << endl << endl;

	// Draw samples from joint distr=ibution using Gibbs sampling
	// and write them to the .tab file
	size_t nrSamples = numSamples;
	std::vector<size_t> state;
	for (size_t t = 0; t < nrSamples; t++) {
		gibbsSampler.init();
		{
			gibbsSampler.run();
		}
		state = gibbsSampler.state();
		for (size_t i = 0; i < state.size(); i++)
			outfile << (i == 0 ? "" : "\t") << state[i];
		outfile << endl;
	}
	cout << nrSamples << " samples written to " << outName << endl;

	// Close the .tab file
	outfile.close();
	return outName;
}

void fixParams(vector<int>* fixedVars, FactorGraph* fg, InfAlg* inf) {
	FactorGraph inf_fg = inf->fg();

	// this is broken...
	for (size_t i = 0; i < fixedVars->size(); i++) {
		int ind = (*fixedVars)[i];
		Factor f = fg->factor(ind);
		inf_fg.setFactor(ind, f, false);
	}
}

void printEMIntermediates(stringstream* s_out, InfAlg* inf) {
	//print intermediate variables in python format
	FactorGraph int_fg = inf->fg();
	vector<Factor> factors = int_fg.factors();
	size_t size = factors.size();
	//*s_out << "[";
	for (size_t i = 0; i < size; i++) {
		Factor fac = int_fg.factor(i);
		size_t var_size = fac.nrStates();

		for (size_t j = 0; j < var_size; j++) {
			*s_out << fac.get(j);
			if (j != var_size - 1)
				*s_out << ", ";
		}
		//*s_out << "]";
		if (i != size - 1) {
			*s_out << ", ";
		}
	}
	//*s_out << "]";
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

void uniformize_fg(FactorGraph* fg) {
	vector<Factor> factors = fg->factors();
	size_t size = factors.size();
	for (size_t i = 0; i < size; i++) {
		Factor f = fg->factor(i);
		f.setUniform();
		fg->setFactor(i, f, false);
	}
}

void noise_fg(FactorGraph* fg) {
	rnd_seed((unsigned) time(NULL));
	srand((unsigned) time(NULL));
	vector<Factor> factors = fg->factors();
	size_t size = factors.size();
	for (size_t i = 0; i < size; i++) {
		Factor f = fg->factor(i);
		for (size_t j = 0; j < f.nrStates(); j++) {
			double init_val = f.get(j);
			double val = ((double) rand() / (double) RAND_MAX);
			val *= NOISE_AMOUNT * 2;
			val -= NOISE_AMOUNT;
			f.set(j, init_val + val * init_val);
		}
		f.normalize(NORMLINF);
		f.normalize(NORMPROB);
		fg->setFactor(i, f, false);
	}
}

inline double compareFG(FactorGraph* fg1, FactorGraph* fg2) {
	size_t numFactors = fg1->nrFactors();
	if (fg2->nrFactors() != numFactors)
		throw "compareFG: factors not equal";
	double sum = 0;
	vector<Factor> ff1 = fg1->factors();
	vector<Factor> ff2 = fg2->factors();

	for (size_t i = 0; i < numFactors; i++) {
		Factor f1 = ff1[i];//fg1->factor(i);
		Factor f2 = ff2[i];//fg2->factor(i);
		sum += dist(f1, f2, DISTL1);
		//cout << sum << endl;
	}
	return sum;
}

inline double KLcompareFG(FactorGraph* fg1, FactorGraph* fg2) {
	size_t numFactors = fg1->nrFactors();
	if (fg2->nrFactors() != numFactors)
		throw "compareFG: factors not equal";
	double sum = 0;
	vector<Factor> ff1 = fg1->factors();
	vector<Factor> ff2 = fg2->factors();

	for (size_t i = 0; i < numFactors; i++) {
		Factor f1 = ff1[i];//fg1->factor(i);
		Factor f2 = ff2[i];//fg2->factor(i);
		sum += dist(f1, f2, DISTKL);
		//cout << sum << endl;
	}
	return sum;
}

void doEm(char* fgIn, char* tabIn, char* emIn, int init) {
	string fixedIn = emIn;

#ifdef OLD_FIXING

	string::size_type pos = 0;
	pos = fixedIn.find(".em", pos);
	if (pos != string::npos)
		fixedIn.replace(pos, fixedIn.size(), ".fixed");

	ifstream ifile(fixedIn.c_str());
	vector<int> fixedVars;

	if (ifile) {
		// The file exists, and is open for input
		cout << "Using fixed values from: " << fixedIn << endl;
		ifstream fin(fixedIn.c_str());
		int var;
		cout << "Fixed params: ";
		while (fin >> var) {
			cout << var << " ";
			fixedVars.push_back(var);
		}
		cout << endl;
		fin.close();
	}
#endif

	FactorGraph fg;
	fg.ReadFromFile(fgIn);

	FactorGraph* fg_orig = fg.clone();

	srand((unsigned) time(NULL));
	rnd_seed((unsigned) time(NULL));

	string outname;
	string outname_l;

#ifdef OLD_FIXING
	if (fixedVars.size() > 0) {
		for (size_t i =0; i<fixedVars.size(); i++)
			fg.backupFactor(fixedVars[i]);
	}
#endif

	if (init == 1) {
		// random
		cout << "Using random initialization" << endl;
		randomize_fg(&fg);
		outname = "out/random";
	} else if (init == 2) {
		// uniform
		cout << "Using uniform initialization" << endl;
		uniformize_fg(&fg);
		outname = "out/uniform";
	} else if (init == 3) {
		// noise
		cout << "Using noisy initialization. Noise value: +/- "
				<< NOISE_AMOUNT * 100 << "%" << endl;
		noise_fg(&fg);
		outname = "out/noise";
	} else {
		cout << "Using given .fg for initialization" << endl;
		outname = "out/default";
	}

#ifdef OLD_FIXING
	if (fixedVars.size() > 0) {
		for (size_t i =0; i<fixedVars.size(); i++) {
			size_t index = fixedVars[i];
			fg.restoreFactor(index);
			//			Factor f = fg.factor(index);
			//			vector<Factor> vec;
			//			vec.push_back(f);
			//fg.clamp(index,index,false);
			//fg.clampFactor(index,vec, false);
		}
	}
#endif
	outname_l = outname;
	outname_l = outname_l.append(".lhood");
	cout << "Writing results to: " << outname.c_str() << endl;
	cout << "Writing likelihoods to: " << outname_l.c_str() << endl;

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
	//compareFG(&fg, inf->fg().clone());
	cout.precision(16);
	stringstream s_out_l;
	stringstream s_out;
	//s_out << "[\n";
	s_out.precision(16);
	s_out_l.precision(16);
	s_out_l << "Iteration\tLikelihood\tError_L1\tLikelihood/n\tKL_error"
			<< endl;
	for (size_t i = 0; i < fg.nrFactors(); i++) {
		s_out << fg.factor(i).nrStates();
		if (i != fg.nrFactors() - 1)
			s_out << ", ";
	}
	s_out << endl;
	// initial values
	printEMIntermediates(&s_out, inf);
	s_out << endl;
	//size_t its = 0;
	// Iterate EM until convergence
	while (!em.hasSatisfiedTermConditions()) {
#ifdef OLD_FIXING
		if (fixedVars.size() > 0) {
			for (size_t i =0; i<fixedVars.size(); i++)
				inf->backupFactor(fixedVars[i]);
		}
#endif
		double l = em.iterate();
		FactorGraph* current_fg = inf->fg().clone();
		// TODO: verify exp is correct. assuming log likelihood.
		cout << "Iteration " << em.Iterations() << " likelihood: " << l
				<< endl;
#ifdef OLD_FIXING
		if (fixedVars.size() > 0) {
			for (size_t i =0; i<fixedVars.size(); i++)
				inf->restoreFactor(fixedVars[i]);
		}
#endif
		double fg_diff = compareFG(current_fg, fg_orig);
		s_out_l << em.Iterations() << "\t" << l << "\t" << fg_diff << "\t"
				<< l / e.nrSamples() << "\t" << KLcompareFG(current_fg,
						fg_orig) << endl;

		printEMIntermediates(&s_out, inf);
		s_out << endl;
		s_out.flush();
		s_out_l.flush();
	}

	// Clean up
	delete inf;
	cout << "Intermediate values: an array of tab delimited K*N "
			<< "where N=numFactors and K=iterations." << endl;
	cout << "Includes initial values (i.e. iteration 0)" << endl;

	ofstream f_outl;
	f_outl.open(outname_l.c_str());
	f_outl << s_out_l.str() << endl;
	f_outl.close();
	cout << s_out_l.str() << endl;
#ifdef INTERMEDIATE_VALUES
	ofstream myfile;
	myfile.open(outname.c_str());
	myfile << s_out.str() << endl;
	myfile.close();
#endif
}

void printUsage() {
	//cout << "Not enough args [" << argc-1 <<"]. Requires 2: an .fg file and .tab file." << endl;
	//cout << "Builtin inference algorithms: " << builtinInfAlgNames() << endl << endl;
	cout << "--- Usage ---" << endl;

	cout << "display fg stats/marginals:\n\t./simple asd.fg" << endl;
	cout
	<< "generate tab file (output will be *.tab): \n\t./simple asd.fg num_samples"
	<< endl;
	cout
	<< "run EM (now w/ intermediate results) \n\t./simple asd.fg asd.tab asd.em"
	<< endl;
	cout
	<< "for different EM initialization types use: -noise, -random, -uniform, -all"
	<< endl;
	cout << "\te.g. ./simple -noise asd.fg asd.tab asd.em" << endl;
	cout << "compare EM files (e.g. for shared/non-shared)\n\t"
			<< "./simple -c asd.fg asd1.em asd2.em" << endl;
	cout << "Run EM with increasing number of samples\n\t"
			<< ".simple -s asd.fg asd.em\n\t"
			<< ".simple -s asd.fg asd.em [python command to run for each set of samples (e.g. to hide samples)]\n\t"
			<< ".simple -s -noise asd.fg asd.em" << endl;
	cout << "Run EM using many random initializations\n\t"
			<< "./simple -many_random asd.fg asd.tab asd.em" << endl;
	cout << endl << "--- Info ---" << endl;
	cout
	<< "If running EM and a *.fixed file exists, where * is the given .em file,\n"
	<< "  integers in the fixed file will be used for parameter fixing."
	<< endl;
	cout << "Results output to \"out/TYPE\"" << endl;
	cout << "Builtin inference algorithms: " << builtinInfAlgNames()
												<< endl;
	cout << "Currently used inference algorithm: " << INF_TYPE << endl;
	//cout << "compare FG files (e.g. checking difference from true vs. EM generated FGs)\n\t" <<
	//	"./simple -f asd.fg asd2.fg" << endl;
}

//
//	".simple -s asd.fg asd.em\n\t" <<
//				".simple -s -noise asd.fg asd.em" << endl;

void printDot(char* fgIn) {
	FactorGraph fg;
	fg.ReadFromFile(fgIn);
	ofstream fout;
	fout.open("fg.dot");
	fg.printDot(fout);
	cout << "Printing to fg.dot" << endl;
}

inline string convertInt(int number) {
	stringstream ss;//create a stringstream
	ss << number;//add number to the stream
	return ss.str();//return a string with the contents of the stream
}

inline string readFile(const char* path) {

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

void doRandomEM(char* fgIn, char* tabIn, char* emIn, size_t rand_trials,
		size_t rand_iterations) {
	vector<int> fixedVars;
#ifdef OLD_FIXING
	string fixedIn = emIn;
	string::size_type pos = 0;
	pos = fixedIn.find(".em", pos);
	if (pos != string::npos)
		fixedIn.replace(pos, fixedIn.size(), ".fixed");

	ifstream ifile(fixedIn.c_str());

	if (ifile) {
		// The file exists, and is open for input
		cout << "Using fixed values from: " << fixedIn << endl;
		ifstream fin(fixedIn.c_str());
		int var;
		cout << "Fixed params: ";
		while (fin >> var) {
			cout << var << " ";
			fixedVars.push_back(var);
		}
		cout << endl;
		fin.close();
	}
#endif
	FactorGraph fg;
	fg.ReadFromFile(fgIn);

	FactorGraph fg_orig = *(fg.clone());

	//string outname;
	stringstream outname_l;
	string emFile = readFile(emIn);

	cout << "Using random initialization" << endl;
	system("mkdir -p out/rand_trials");
	//outname = "out/random_trials";

	string trials_dir = "out/rand_trials/";
	cout.precision(16);

	outname_l << "out/random_" << rand_trials << "trials_"
			<< rand_iterations << "iterations";
	stringstream s_out_l;
	s_out_l << "Iterations\tLikelihood\tL1_error\tLikelihood/n\tKL_error"
			<< endl;
	cout << "Writing results to: " << outname_l.str().c_str() << endl;

	Evidence e1;
	//string tabFile = readFile(tabIn);
	//stringstream estream(tabFile);

	ifstream estream(tabIn);
	e1.addEvidenceTabFile(estream, fg);
	typedef std::map<Var, size_t> Observation;
	vector<Observation> samples = e1.getEvidence();

	for (size_t trials = 1; trials <= rand_trials; trials++) {
		srand((unsigned) time(NULL));
		rnd_seed((unsigned) time(NULL));
		stringstream trials_out;
		trials_out
		<< "Iteration\tLikelihood\tL1_error\tLikelihood/n\tKL_error"
		<< endl;

		cout << "random trial: " << trials << endl;
		fg = *(fg_orig.clone());
#ifdef OLD_FIXING
		if (fixedVars.size() > 0) {
			for (size_t i =0; i<fixedVars.size(); i++)
				fg.backupFactor(fixedVars[i]);
		}
#endif
		randomize_fg(&fg);
#ifdef OLD_FIXING
		if (fixedVars.size() > 0) {
			for (size_t i =0; i<fixedVars.size(); i++)
				fg.restoreFactor(fixedVars[i]);
		}
#endif
		// Read sample from file

		//ifstream estream( tabIn );

		Evidence e(samples);

		// Prepare junction-tree object for doing exact inference for E-step
		PropertySet infprops;
		infprops.set("verbose", (size_t) 1);
		infprops.set("updates", string("HUGIN"));
		InfAlg* inf = newInfAlg(INF_TYPE, fg, infprops);
		inf->init();
		// Read EM specification
		//ifstream emstream( emIn );
		stringstream emstream(emFile);
		EMAlg em(e, *inf, emstream);
		//compareFG(&fg, inf->fg().clone());

		trials_out.precision(16);

		double l = 99999999;
		// Iterate EM until convergence

		//while( ++i <= rand_iterations ) {
		while (!em.hasSatisfiedTermConditions()) {
			if (fixedVars.size() > 0) {
				for (size_t i = 0; i < fixedVars.size(); i++)
					inf->backupFactor(fixedVars[i]);
			}

			l = em.iterate();

			if (fixedVars.size() > 0) {
				for (size_t i = 0; i < fixedVars.size(); i++)
					inf->restoreFactor(fixedVars[i]);
			}

			FactorGraph* current_fg = inf->fg().clone();
			double fg_diff = compareFG(current_fg, &fg_orig);
			trials_out << em.Iterations() << "\t" << l << "\t" << fg_diff
					<< "\t" << l / e.nrSamples() << "\t" << KLcompareFG(
							current_fg, &fg_orig) << endl;

		}

		ofstream f_outl2;
		string temp_s = trials_dir;
		f_outl2.open(temp_s.append(convertInt(trials)).c_str());
		f_outl2 << trials_out.str() << endl;
		f_outl2.close();

		FactorGraph* current_fg = inf->fg().clone();
		double fg_diff = compareFG(current_fg, &fg_orig);
		double kl_diff = KLcompareFG(current_fg, &fg_orig);

		{
			s_out_l.precision(16);
			s_out_l << em.Iterations() << "\t" << l << "\t" << fg_diff
					<< "\t" << l / e.nrSamples() << "\t" << kl_diff << endl;
		}
		// Clean up
		delete inf;
	}
	ofstream f_outl;
	f_outl.open(outname_l.str().c_str());
	f_outl << s_out_l.str() << endl;
	f_outl.close();

}

bool str_replace(std::string& str, const std::string& from,
		const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

void doEmSamples(char* fgIn, char* emIn, int init, string py_cmd) {
#ifdef OLD_FIXING
	string fixedIn = emIn;
	string::size_type pos = 0;
	pos = fixedIn.find(".em", pos);
	if (pos != string::npos)
		fixedIn.replace(pos, fixedIn.size(), ".fixed");

	ifstream ifile(fixedIn.c_str());
	vector<int> fixedVars;

	if (ifile) {
		// The file exists, and is open for input
		cout << "Using fixed values from: " << fixedIn << endl;
		ifstream fin(fixedIn.c_str());
		int var;
		cout << "Fixed params: ";
		while (fin >> var) {
			cout << var << " ";
			fixedVars.push_back(var);
		}
		cout << endl;
		fin.close();
	}
#endif
	FactorGraph fg;
	fg.ReadFromFile(fgIn);

	FactorGraph fg_orig = *fg.clone();
	string emFile = readFile(emIn);

	srand((unsigned) time(NULL));
	rnd_seed((unsigned) time(NULL));
	string outname;

#ifdef OLD_FIXING
	if (fixedVars.size() > 0) {
		for (size_t i =0; i<fixedVars.size(); i++)
			fg.backupFactor(fixedVars[i]);
	}
#endif
	if (init == 1) {
		// random
		cout << "Using random initialization" << endl;
		randomize_fg(&fg);
		outname = "out/random.many_samples";
	} else if (init == 2) {
		// uniform
		cout << "Using uniform initialization" << endl;
		uniformize_fg(&fg);
		outname = "out/uniform.many_samples";
	} else if (init == 3) {
		// noise
		cout << "Using noisy initialization. Noise value: +/- "
				<< NOISE_AMOUNT * 100 << "%" << endl;
		noise_fg(&fg);
		outname = "out/noise.many_samples";
	} else {
		cout << "Using given .fg for initialization" << endl;
		outname = "out/default.many_samples";
	}
#ifdef OLD_FIXING
	if (fixedVars.size() > 0) {
		for (size_t i =0; i<fixedVars.size(); i++)
			fg.restoreFactor(fixedVars[i]);
	}
#endif
	cout << "Initial samples: " << EM_INIT_SAMPLES << endl;
	cout << "Max samples: " << EM_MAX_SAMPLES << ". Increase by "
			<< EM_SAMPLES_DELTA << endl;
	cout << "Writing results to: " << outname.c_str() << endl;

	ofstream fout;
	fout.open(outname.c_str());
	fout.precision(12);
	fout << "numSamples\tlikelihood\titerations\terror" << endl;

	// TODO: fixed params not local in this loop
	for (size_t i = EM_INIT_SAMPLES; i <= EM_MAX_SAMPLES; i
	+= EM_SAMPLES_DELTA) {
		FactorGraph local_fg = *fg.clone();
		if (init == 1)
			randomize_fg(&local_fg);
		else if (init == 3) {
			noise_fg(&local_fg);
		}

		PropertySet infprops;
		infprops.set("verbose", (size_t) 1);
		infprops.set("updates", string("HUGIN"));
		infprops.set("maxiter", EM_MAX_ITER); // Maximum number of iterations
		infprops.set("tol", LIB_EM_TOLERANCE);

		InfAlg* inf1 = newInfAlg(INF_TYPE, local_fg, infprops);
		inf1->init();

		string tabIn;
		stringstream newTabName;
		stringstream hiddenTabName;
		if (py_cmd != "NULL") {
			string py_cmd_str = py_cmd;
			newTabName << fgIn << i;
			hiddenTabName << fgIn << i << ".hidden";
			generateTab(fgIn, i, newTabName.str().c_str());

			if (!str_replace(py_cmd_str, "TAB_PATH", newTabName.str())) {
				cerr << "TAB_PATH not found in PY_CMD" << endl;
				throw "TAB_PATH not found in PY_CMD";
			}
			if (!str_replace(py_cmd_str, "HIDDEN_PATH", hiddenTabName.str())) {
				cerr << "HIDDEN_PATH not found in PY_CMD" << endl;
				throw "HIDDEN_PATH not found in PY_CMD";
			}
			tabIn = hiddenTabName.str();

			cout << "Executing command: " << py_cmd_str << endl;
			system(py_cmd_str.c_str());
		} else
			tabIn = generateTab(fgIn, i, NULL);
		Evidence e;

		ifstream estream(tabIn.c_str());

		e.addEvidenceTabFile(estream, fg);
		estream.close();
		if (py_cmd != "NULL") {
			//delete samples created
			if (remove(hiddenTabName.str().c_str()) != 0)
				cerr << "error deleting " << hiddenTabName << endl;
			if (remove(newTabName.str().c_str()) != 0)
				cerr << "error deleting " << newTabName << endl;
		}

		// Read EM specification

		stringstream emstream(emFile);
		EMAlg em(e, *inf1, emstream);
		em.setTermConditions(infprops);
		Real l1;
		// Iterate EM until convergence
		while (!em.hasSatisfiedTermConditions()) {
#ifdef OLD_FIXING
			if (fixedVars.size() > 0) {
				for (size_t i =0; i<fixedVars.size(); i++)
					inf1->backupFactor(fixedVars[i]);
			}
#endif
			l1 = em.iterate();

			//			cout << "em1: Iteration " << em1.Iterations() << " likelihood: " << l1
			//					<< endl;
#ifdef OLD_FIXING
			if (fixedVars.size() > 0) {
				for (size_t i =0; i<fixedVars.size(); i++)
					inf1->restoreFactor(fixedVars[i]);
			}
#endif
		}
		double fg_diff = compareFG(&inf1->fg(), &fg_orig);
		{
			fout << e.nrSamples() << "\t" << l1 << "\t" << em.Iterations()
														<< "\t" << fg_diff << endl;
		}
		delete inf1;
	}
	fout.close();
}

std::vector<std::string> &str_split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
        elems.push_back(item);
    return elems;
}


std::vector<std::string> str_split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return str_split(s, delim, elems);
}

//Real EMAlg::hadoop_expectation( MaximizationStep &mstep ) {
//    Real logZ = 0;
//    Real likelihood = 0;
//
//    _estep.run();
//    logZ = _estep.logZ();
//
//    // Expectation calculation
//    for( Evidence::const_iterator e = _evidence.begin(); e != _evidence.end(); ++e ) {
//        InfAlg* clamped = _estep.clone();
//        // Apply evidence
//        for( Evidence::Observation::const_iterator i = e->begin(); i != e->end(); ++i )
//            clamped->clamp( clamped->fg().findVar(i->first), i->second );
//        clamped->init();
//        clamped->run();
//
//        likelihood += clamped->logZ() - logZ;
//
//        mstep.addExpectations( *clamped );
//
//        delete clamped;
//    }
//
//    // Maximization of parameters
////    mstep.maximize( _estep.fg() );
//
//    return likelihood;
//}

//Real EMAlg::hadoop_maximization(Real likelihood, MaximizationStep &mstep ) {
//
//    // Maximization of parameters
//    mstep.maximize( _estep.fg() );
//
//    return likelihood;
//}
//
//
// Real EMAlg::hadoop_iterate() {
//    Real likelihood;
//    for( size_t i = 0; i < _msteps.size(); ++i )
//        likelihood = iterate( _msteps[i] );
//    _lastLogZ.push_back( likelihood );
//    ++_iters;
//    return likelihood;
//}

void mapper(const string& in,vector<string>& key,vector<string>& val) {

	vector<string> data = str_split(in, '*');
	if (data.size() != 3)
		throw -3;
	string emFile= data[0];
	string fgFile= data[1];
	string tabFile= data[2];
	cout << tabFile.size() << endl;

	ostringstream s;
	MaximizationStep m;
	boost::archive::text_oarchive oa(s);
	oa << m;
//	return NULL;
}

string reduce(vector<string>& key, vector<string>& val) {

	return NULL;
}


int main(int argc, char* argv[]) {

	size_t numMappers = 5;

	string emFile= readFile("dat/em");
	string fgFile= readFile("dat/fg");
	string tabFile= readFile("dat/tab");

	vector<string> tabLines = str_split(tabFile, '\n');
//	numMappers = tabLines.size()/numMappers > numMappers ? numMappers : tabLines.size()/numMappers;

	size_t samplesPerMapper = tabLines.size()/numMappers;

	vector<string> mapDatForReducer;
	vector<string> redDatForReducer;

	size_t t = 0;
	for (size_t i=0; i<numMappers; i++) {
		stringstream datForMapper;
		datForMapper << emFile << "*" << fgFile << "*";
		for (size_t j=0; j<samplesPerMapper && t<tabLines.size(); j++){
			datForMapper << tabLines[t++] << '\n';
		}
		string s = datForMapper.str();
		mapper(s,mapDatForReducer,redDatForReducer);
	}

	return 0;
}



