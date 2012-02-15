// erik reed
// erikreed@cmu.edu

#include <dai/alldai.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <fstream>
#include <math.h>
#include <omp.h>
#include <stdio.h>

using namespace std;
using namespace dai;
// TODO: make random initialization pragma critical
//#define DO_RANDOM_EM // does the many iterations of random EM
#define INTERMEDIATE_VALUES
#define NOISE_AMOUNT .05 // corresponding to 5%
#define INF_TYPE "JTREE"

// constants for compareEM(...)
const size_t EM_MAX_SAMPLES =1600;
const size_t EM_SAMPLES_DELTA= 2; // now set to double each time
const size_t EM_INIT_SAMPLES =100;

const Real LIB_EM_TOLERANCE = 1e-4;
const size_t EM_MAX_ITER = 100;
//#define OLD_FIXING

const size_t RANDOM_EM_TRIALS = 50;
const size_t RANDOM_EM_ITERATIONS = 100; //unused now
const size_t VERBOSE = 0;
void displayStats(char** argv) {
	//Builtin inference algorithms: {BP, CBP, DECMAP, EXACT, FBP, GIBBS, HAK, JTREE, LC, MF, MR, TREEEP, TRWBP}

	FactorGraph fg;
	fg.ReadFromFile( argv[1] );

	cout << fg.nrVars() << " variables" << endl;
	cout << fg.nrFactors() << " factors" << endl;
	cout << endl << "Given factor graph:" << endl << "##################" << endl;
	cout.precision(12);
	cout << fg;



	size_t maxstates = 1000000;
	size_t maxiter = 10000;
	Real   tol = 1e-9;
	size_t verb = 1;

	PropertySet opts;
	opts.set("maxiter",maxiter);  // Maximum number of iterations
	opts.set("tol",tol);          // Tolerance for convergence
	opts.set("verbose",verb);     // Verbosity (amount of output generated)

	// Bound treewidth for junctiontree
	bool do_jt = true;
	try {
		boundTreewidth(fg, &eliminationCost_MinFill, maxstates );
	} catch( Exception &e ) {
		if( e.getCode() == Exception::OUT_OF_MEMORY ) {
			do_jt = false;
			cout << "Skipping junction tree (need more than " << maxstates << " states)." << endl;
		}
		else
			throw;
	}

	JTree jt, jtmap;
	vector<size_t> jtmapstate;
	if( do_jt ) {
		// Construct a JTree (junction tree) object from the FactorGraph fg
		// using the parameters specified by opts and an additional property
		// that specifies the type of updates the JTree algorithm should perform
		jt = JTree( fg, opts("updates",string("HUGIN")) );
		// Initialize junction tree algorithm
		jt.init();
		// Run junction tree algorithm
		jt.run();

		// Construct another JTree (junction tree) object that is used to calculate
		// the joint configuration of variables that has maximum probability (MAP state)
		jtmap = JTree( fg, opts("updates",string("HUGIN"))("inference",string("MAXPROD")) );
		// Initialize junction tree algorithm

		jtmap.init();
		// Run junction tree algorithm
		jtmap.run();
		// Calculate joint state of all variables that has maximum probability
		jtmapstate = jtmap.findMaximum();
	}

	// Construct a BP (belief propagation) object from the FactorGraph fg
	// using the parameters specified by opts and two additional properties,
	// specifying the type of updates the BP algorithm should perform and
	// whether they should be done in the real or in the logdomain
	BP bp(fg, opts("updates",string("SEQRND"))("logdomain",false));
	// Initialize belief propagation algorithm
	bp.init();
	// Run belief propagation algorithm
	bp.run();

	// Construct a BP (belief propagation) object from the FactorGraph fg
	// using the parameters specified by opts and two additional properties,
	// specifying the type of updates the BP algorithm should perform and
	// whether they should be done in the real or in the logdomain
	//
	// Note that inference is set to MAXPROD, which means that the object
	// will perform the max-product algorithm instead of the sum-product algorithm
	BP mp(fg, opts("updates",string("SEQRND"))("logdomain",false)("inference",string("MAXPROD"))("damping",string("0.1")));
	// Initialize max-product algorithm
	mp.init();
	// Run max-product algorithm
	mp.run();
	// Calculate joint state of all variables that has maximum probability
	// based on the max-product result
	vector<size_t> mpstate = mp.findMaximum();  //fails for adapt.fg

	// Construct a decimation algorithm object from the FactorGraph fg
	// using the parameters specified by opts and three additional properties,
	// specifying that the decimation algorithm should use the max-product
	// algorithm and should completely reinitalize its state at every step
	DecMAP decmap(fg, opts("reinit",true)("ianame",string("BP"))("iaopts",string("[damping=0.1,inference=MAXPROD,logdomain=0,maxiter=1000,tol=1e-9,updates=SEQRND,verbose=1]")) );
	decmap.init();
	decmap.run();
	vector<size_t> decmapstate = decmap.findMaximum();

	if( do_jt ) {
		// Report variable marginals for fg, calculated by the junction tree algorithm
		cout << "Exact variable marginals:" << endl;
		for( size_t i = 0; i < fg.nrVars(); i++ ) // iterate over all variables in fg
			cout << jt.belief(fg.var(i)) << endl; // display the "belief" of jt for that variable
	}

	// Report variable marginals for fg, calculated by the belief propagation algorithm
	cout << "Approximate (loopy belief propagation) variable marginals:" << endl;
	for( size_t i = 0; i < fg.nrVars(); i++ ) // iterate over all variables in fg
		cout << bp.belief(fg.var(i)) << endl; // display the belief of bp for that variable

	if( do_jt ) {
		// Report factor marginals for fg, calculated by the junction tree algorithm
		cout << "Exact factor marginals:" << endl;
		for( size_t I = 0; I < fg.nrFactors(); I++ ) // iterate over all factors in fg
			cout << jt.belief(fg.factor(I).vars()) << endl;  // display the "belief" of jt for the variables in that factor
	}


	// Report log partition sum of fg, approximated by the belief propagation algorithm
	cout << "Approximate (loopy belief propagation) log partition sum: " << bp.logZ() << endl;

	if( do_jt ) {
		// Report exact MAP variable marginals
		cout << "Exact MAP variable marginals:" << endl;
		for( size_t i = 0; i < fg.nrVars(); i++ )
			cout << jtmap.belief(fg.var(i)) << endl;
	}

	// Report max-product variable marginals
	cout << "Approximate (max-product) MAP variable marginals:" << endl;
	for( size_t i = 0; i < fg.nrVars(); i++ )
		cout << mp.belief(fg.var(i)) << endl;

	if( do_jt ) {
		// Report exact MAP factor marginals
		cout << "Exact MAP factor marginals:" << endl;
		for( size_t I = 0; I < fg.nrFactors(); I++ )
			cout << jtmap.belief(fg.factor(I).vars()) << " == " << jtmap.beliefF(I) << endl;
	}

	// Report max-product factor marginals
	cout << "Approximate (max-product) MAP factor marginals:" << endl;
	for( size_t I = 0; I < fg.nrFactors(); I++ )
		cout << mp.belief(fg.factor(I).vars()) << " == " << mp.beliefF(I) << endl;

	if( do_jt ) {
		// Report exact MAP joint state
		cout << "Exact MAP state (log score = " << fg.logScore( jtmapstate ) << "):" << endl;
		for( size_t i = 0; i < jtmapstate.size(); i++ )
			cout << fg.var(i) << ": " << jtmapstate[i] << endl;
	}
	// Report max-product MAP joint state
	cout << "Approximate (max-product) MAP state (log score = " << fg.logScore( mpstate ) << "):" << endl;
	for( size_t i = 0; i < mpstate.size(); i++ )
		cout << fg.var(i) << ": " << mpstate[i] << endl;

	// Report DecMAP joint state
	cout << "Approximate DecMAP state (log score = " << fg.logScore( decmapstate ) << "):" << endl;
	for( size_t i = 0; i < decmapstate.size(); i++ )
		cout << fg.var(i) << ": " << decmapstate[i] << endl;

}

string generateTab(const char* input, int numSamples, const char* custom_out) {
	string outName = input;
	if (custom_out != NULL) {
		outName = custom_out;
	}
	else {
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
	factorIn.ReadFromFile( input );
	cout << "Reading " << input << endl;

	// Output some information about the factorgraph
	cout << factorIn.nrVars() << " variables" << endl;
	cout << factorIn.nrFactors() << " factors" << endl;
	cout << "Generating " << outName << endl;
	cout << "Number of samples: " << numSamples << endl;

	srand ( time(NULL) ); // set random seed

	// Prepare a Gibbs sampler
	PropertySet gibbsProps;
	gibbsProps.set("maxiter", size_t(numSamples));   // number of Gibbs sampler iterations
	gibbsProps.set("burnin", size_t(0));
	gibbsProps.set("verbose", VERBOSE);
	Gibbs gibbsSampler( factorIn, gibbsProps );

	// Open a .tab file for writing
	ofstream outfile;

	outfile.open( outName.c_str());
	if( !outfile.is_open() )
		throw "Cannot write to file!";

	// Write header (consisting of variable labels)
	for( size_t i = 0; i < factorIn.nrVars(); i++ )
		outfile << (i == 0 ? "" : "\t") << factorIn.var(i).label();
	outfile << endl << endl;

	// Draw samples from joint distr=ibution using Gibbs sampling
	// and write them to the .tab file
	size_t nrSamples = numSamples;
	std::vector<size_t> state;
	for( size_t t = 0; t < nrSamples; t++ ) {
		gibbsSampler.init();
		gibbsSampler.run();
		state = gibbsSampler.state();
		for( size_t i = 0; i < state.size(); i++ )
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
	for (size_t i=0; i< fixedVars->size(); i++) {
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
	for (size_t i = 0; i<size; i++) {
		Factor fac = int_fg.factor(i);
		size_t var_size = fac.nrStates();

		for (size_t j=0; j<var_size; j++) {
			*s_out << fac.get(j);
			if (j != var_size-1)
				*s_out << ", ";
		}
		//*s_out << "]";
		if (i != size-1){
			*s_out << ", ";
		}
	}
	//*s_out << "]";
}

inline void randomize_fg(FactorGraph* fg) {
	srand((unsigned)time(NULL));
	rnd_seed((unsigned)time(NULL));
	vector<Factor> factors = fg->factors();
	size_t size = factors.size();
	for (size_t i = 0; i<size; i++) {
		Factor f = fg->factor(i);
		f.randomize();
		fg->setFactor(i,f,false);
	}
}

void uniformize_fg(FactorGraph* fg) {
	vector<Factor> factors = fg->factors();
	size_t size = factors.size();
	for (size_t i = 0; i<size; i++) {
		Factor f = fg->factor(i);
		f.setUniform();
		fg->setFactor(i,f,false);
	}
}

void noise_fg(FactorGraph* fg) {
	rnd_seed((unsigned)time(NULL));
	srand((unsigned)time(NULL));
	vector<Factor> factors = fg->factors();
	size_t size = factors.size();
	for (size_t i = 0; i<size; i++) {
		Factor f = fg->factor(i);
		for (size_t j = 0; j < f.nrStates(); j++) {
			double init_val = f.get(j);
			double val = ((double)rand()/(double)RAND_MAX);
			val *= NOISE_AMOUNT*2;
			val -= NOISE_AMOUNT;
			f.set(j, init_val+val*init_val);
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
	double sum=0;
	vector<Factor> ff1 = fg1->factors();
	vector<Factor> ff2 = fg2->factors();

	for (size_t i=0; i< numFactors; i++) {
		Factor f1 = ff1[i];//fg1->factor(i);
		Factor f2 = ff2[i];//fg2->factor(i);
		sum += dist(f1,f2, DISTL1);
		//cout << sum << endl;
	}
	return sum;
}

inline double KLcompareFG(FactorGraph* fg1, FactorGraph* fg2) {
	size_t numFactors = fg1->nrFactors();
	if (fg2->nrFactors() != numFactors)
		throw "compareFG: factors not equal";
	double sum=0;
	vector<Factor> ff1 = fg1->factors();
	vector<Factor> ff2 = fg2->factors();

	for (size_t i=0; i< numFactors; i++) {
		Factor f1 = ff1[i];//fg1->factor(i);
		Factor f2 = ff2[i];//fg2->factor(i);
		sum += dist(f1,f2, DISTKL);
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
			cout  << var << " ";
			fixedVars.push_back(var);
		}
		cout << endl;
		fin.close();
	}
#endif

	FactorGraph fg;
	fg.ReadFromFile( fgIn );

	FactorGraph* fg_orig = fg.clone();

	srand((unsigned)time(NULL));
	rnd_seed((unsigned)time(NULL));

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
	}
	else if (init == 2) {
		// uniform
		cout << "Using uniform initialization" << endl;
		uniformize_fg(&fg);
		outname = "out/uniform";
	}
	else if (init == 3) {
		// noise
		cout << "Using noisy initialization. Noise value: +/- " <<
				NOISE_AMOUNT*100 << "%" << endl;
		noise_fg(&fg);
		outname = "out/noise";
	}
	else {
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
	infprops.set( "verbose", VERBOSE );
	infprops.set( "updates", string("HUGIN") );
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("LOG_Z_TOL_KEY", LIB_EM_TOLERANCE);
	infprops.set(EMAlg::LOG_Z_TOL_KEY, LIB_EM_TOLERANCE);

	InfAlg* inf = newInfAlg( INF_TYPE, fg, infprops );
	inf->init();

	// Read sample from file
	Evidence e;
	ifstream estream( tabIn );
	e.addEvidenceTabFile( estream, fg );

	cout << "Number of samples: " << e.nrSamples() << endl;

	// Read EM specification
	ifstream emstream( emIn );
	EMAlg em(e, *inf, emstream);
	em.setTermConditions(infprops);
	em._log_z_tol = LIB_EM_TOLERANCE;
	em._max_iters = EM_MAX_ITER;
	//compareFG(&fg, inf->fg().clone());
	cout.precision(16);
	stringstream s_out_l;
	stringstream s_out;
	//s_out << "[\n";
	s_out.precision(16);
	s_out_l.precision(16);
	s_out_l << "Iteration\tLikelihood\tError_L1\tLikelihood/n\tKL_error" << endl;
	for (size_t i=0; i< fg.nrFactors(); i++) {
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
	while( !em.hasSatisfiedTermConditions() ) {
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
		s_out_l << em.Iterations() << "\t" << l << "\t" << 
				fg_diff << "\t" << l/e.nrSamples() <<
				"\t" << KLcompareFG(current_fg, fg_orig) << endl;

		printEMIntermediates(&s_out, inf);
		s_out << endl;
		s_out.flush();
		s_out_l.flush();
	}

	// Clean up
	delete inf;
	cout << "Intermediate values: an array of tab delimited K*N " <<
			"where N=numFactors and K=iterations." << endl;
	cout << "Includes initial values (i.e. iteration 0)" << endl;

	ofstream f_outl;
	f_outl.open(outname_l.c_str());
	f_outl << s_out_l.str() << endl;
	f_outl.close();
	cout << s_out_l.str() << endl;
#ifdef INTERMEDIATE_VALUES
	ofstream myfile;
	myfile.open (outname.c_str());
	myfile << s_out.str() << endl;
	myfile.close();
#endif
}

void printUsage() {
	//cout << "Not enough args [" << argc-1 <<"]. Requires 2: an .fg file and .tab file." << endl;
	//cout << "Builtin inference algorithms: " << builtinInfAlgNames() << endl << endl;
	cout << "--- Usage ---" << endl;

	cout << "display fg stats/marginals:\n\t./simple asd.fg" << endl;
	cout << "generate tab file (output will be *.tab): \n\t./simple asd.fg num_samples" << endl;
	cout << "run EM (now w/ intermediate results) \n\t./simple asd.fg asd.tab asd.em" << endl;
	cout << "for different EM initialization types use: -noise, -random, -uniform, -all" << endl;
	cout << "\te.g. ./simple -noise asd.fg asd.tab asd.em" << endl;
	cout << "compare EM files (e.g. for shared/non-shared)\n\t" << 
			"./simple -c asd.fg asd1.em asd2.em" << endl;
	cout << "Run EM with increasing number of samples\n\t" <<
			".simple -s asd.fg asd.em\n\t" <<
			".simple -s asd.fg asd.em [python command to run for each set of samples (e.g. to hide samples)]\n\t" <<
			".simple -s -noise asd.fg asd.em" << endl;
	cout << "Run EM using many random initializations\n\t" <<
			"./simple -many_random asd.fg asd.tab asd.em" << endl;
	cout << endl << "--- Info ---" << endl;
	cout << "If running EM and a *.fixed file exists, where * is the given .em file,\n" <<
			"  integers in the fixed file will be used for parameter fixing." << endl;
	cout << "Results output to \"out/TYPE\"" << endl;
	cout << "Builtin inference algorithms: " << builtinInfAlgNames() << endl;
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

size_t count_lines(const char* path) {
	size_t number_of_lines = 0;
	std::string line;
	std::ifstream myfile(path);

	while (std::getline(myfile, line))
		++number_of_lines;
	myfile.close();
	return number_of_lines;
}

inline string readFile(char* path) {

	int length;
	char * buffer;

	ifstream is;
	is.open (path, ios::binary );

	// get length of file:
	is.seekg (0, ios::end);
	length = is.tellg();
	is.seekg (0, ios::beg);

	// allocate memory:
	buffer = new char [length];

	// read data as a block:
	is.read (buffer,length);
	is.close();
	string out = buffer;
	delete[] buffer;
	return out;
}

void doRandomEM(char* fgIn, char* tabIn, char* emIn, size_t rand_trials, size_t rand_iterations) {
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
			cout  << var << " ";
			fixedVars.push_back(var);
		}
		cout << endl;
		fin.close();
	}
#endif
	FactorGraph fg;
	fg.ReadFromFile( fgIn );

	FactorGraph fg_orig = *(fg.clone());

	//string outname;
	stringstream outname_l;
	string emFile = readFile(emIn);

	cout << "Using random initialization" << endl;
	system("mkdir -p out/rand_trials");
	//outname = "out/random_trials";

	string trials_dir = "out/rand_trials/";
	cout.precision(16);

	outname_l << "out/random_" << rand_trials <<"trials_" << rand_iterations << "iterations";
	stringstream s_out_l;
	s_out_l << "Iterations\tLikelihood\tL1_error\tLikelihood/n\tKL_error" << endl;
	cout << "Writing results to: " << outname_l.str().c_str() << endl;

	Evidence e1;
	//string tabFile = readFile(tabIn);
	//stringstream estream(tabFile);

	ifstream estream( tabIn );
	e1.addEvidenceTabFile( estream, fg );
	typedef std::map<Var, size_t> Observation;
	vector<Observation> samples = e1.getEvidence();

	srand((unsigned)time(NULL));
	rnd_seed((unsigned)time(NULL));

#pragma omp parallel for \
		private(fg) \
		schedule(static, 4)
	//firstprivate(rand_trials,fg_orig, trials_dir, fixedVars,emFile, samples)
	for (size_t trials=1; trials<=rand_trials; trials++) {

		stringstream trials_out;
		trials_out << "Iteration\tLikelihood\tL1_error\tLikelihood/n\tKL_error" << endl;

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
		infprops.set( "verbose", VERBOSE );
		infprops.set( "updates", string("HUGIN") );
		InfAlg* inf = newInfAlg( INF_TYPE, fg, infprops );
		inf->init();
		// Read EM specification
		//ifstream emstream( emIn );
		stringstream emstream(emFile);
		EMAlg em(e, *inf, emstream);
		em._log_z_tol = LIB_EM_TOLERANCE;
		em._max_iters = EM_MAX_ITER;
		//compareFG(&fg, inf->fg().clone());

		trials_out.precision(16);

		double l = 99999999;
		// Iterate EM until convergence

		//while( ++i <= rand_iterations ) {
		while( !em.hasSatisfiedTermConditions() ) {
			if (fixedVars.size() > 0) {
				for (size_t i =0; i<fixedVars.size(); i++)
					inf->backupFactor(fixedVars[i]);
			}

			l = em.iterate();

			if (fixedVars.size() > 0) {
				for (size_t i =0; i<fixedVars.size(); i++)
					inf->restoreFactor(fixedVars[i]);
			}

			FactorGraph* current_fg = inf->fg().clone();
			double fg_diff = compareFG(current_fg, &fg_orig);
			trials_out << em.Iterations() << "\t" << l << "\t" <<
					fg_diff << "\t" << l/e.nrSamples() <<
					"\t" << KLcompareFG(current_fg, &fg_orig) << endl;

		}

		ofstream f_outl2;
		string temp_s = trials_dir;
		f_outl2.open(temp_s.append(convertInt(trials)).c_str());
		f_outl2 << trials_out.str() << endl;
		f_outl2.close();

		FactorGraph* current_fg = inf->fg().clone();
		double fg_diff = compareFG(current_fg, &fg_orig);
		double kl_diff = KLcompareFG(current_fg, &fg_orig);

#pragma omp critical
		{
			s_out_l.precision(16);
			s_out_l << em.Iterations() << "\t" << l << "\t" <<
					fg_diff << "\t" << l/e.nrSamples() <<
					"\t" << kl_diff << endl;
		}
		// Clean up
		delete inf;
	}
	ofstream f_outl;
	f_outl.open(outname_l.str().c_str());
	f_outl << s_out_l.str() << endl;
	f_outl.close();

}

bool str_replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos)
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
			cout  << var << " ";
			fixedVars.push_back(var);
		}
		cout << endl;
		fin.close();
	}
#endif
	FactorGraph fg;
	fg.ReadFromFile( fgIn );

	FactorGraph fg_orig = *fg.clone();
	string emFile = readFile(emIn);

	srand((unsigned)time(NULL));
	rnd_seed((unsigned)time(NULL));
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
	}
	else if (init == 2) {
		// uniform
		cout << "Using uniform initialization" << endl;
		uniformize_fg(&fg);
		outname = "out/uniform.many_samples";
	}
	else if (init == 3) {
		// noise
		cout << "Using noisy initialization. Noise value: +/- " <<
				NOISE_AMOUNT*100 << "%" << endl;
		noise_fg(&fg);
		outname = "out/noise.many_samples";
	}
	else {
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
	cout << "Max samples: " << EM_MAX_SAMPLES << ". Increase by " <<
			EM_SAMPLES_DELTA << endl;
	cout << "Writing results to: " << outname.c_str() << endl;
	/*
	ofstream fout;
	fout.open (outname.c_str());
	fout.precision(12);
	fout << "numSamples\tlikelihood\titerations\terror" << endl;
	 */
	// check if .tab exists
	//scope hack
	string::size_type pos = 0;
	string newTabIn = fgIn;
	pos = newTabIn.find(".fg", pos);
	if (pos != string::npos)
		newTabIn.replace(pos, newTabIn.size(), ".tab");
	else
		newTabIn.append(".tab");
	ifstream ifile(newTabIn.c_str());

	if (ifile) {
		// check size. if too small, regen file
		if (count_lines(newTabIn.c_str()) < EM_MAX_SAMPLES)
			generateTab(fgIn, EM_MAX_SAMPLES,NULL);
	}
	else {

		str_replace(newTabIn, "fg/", "tab/");
		ifstream ifile2(newTabIn.c_str());

		if (ifile2) {
			if (count_lines(newTabIn.c_str()) < EM_MAX_SAMPLES)
				generateTab(fgIn, EM_MAX_SAMPLES,NULL);
		}
		else
			generateTab(fgIn, EM_MAX_SAMPLES,newTabIn.c_str());
	}


	string tabIn;
	stringstream hiddenTabName;
	stringstream tempTab;
	if (py_cmd != "NULL") {
		string py_cmd_str = py_cmd;
		hiddenTabName << newTabIn << ".hidden";
		//generateTab(fgIn, i, newTabName.str().c_str());
		//tempTab << newTabIn << i;
		if (!str_replace(py_cmd_str, "TAB_PATH", newTabIn)) {
			cerr << "TAB_PATH not found in PY_CMD" << endl;
			throw "TAB_PATH not found in PY_CMD";
		}
		if (!str_replace(py_cmd_str, "HIDDEN_PATH", hiddenTabName.str())) {
			cerr << "HIDDEN_PATH not found in PY_CMD" << endl;
			throw "HIDDEN_PATH not found in PY_CMD";
		}
		tabIn = hiddenTabName.str();
		//				stringstream awk_cmd;
		//				awk_cmd << "./get_n_lines.sh " << newTabIn <<
		//						" " << tempTab.str() << " " << i;
		//				stringstream rm_blanks_cmd;
		//				rm_blanks_cmd << "sed '/^$/d' " << tabIn << "> tmp" << i <<
		//						" && mv tmp" << i <<" " << tabIn << " && echo -e \"\n\" >> "
		//						<< tabIn;
		//				cout << "Executing command: " << awk_cmd.str() <<endl;
		//				system(awk_cmd.str().c_str());
		cout << "Executing command: " << py_cmd_str << endl;
		system(py_cmd_str.c_str());
		//			cout << "Executing command: " << rm_blanks_cmd.str() <<endl;
		//			system(rm_blanks_cmd.str().c_str());
	}
	else
		tabIn = newTabIn;
	Evidence e1;

	ifstream estream2( tabIn.c_str() );

	system("mkdir -p out/rand_trials");
	string trials_dir = "out/rand_trials/";

	e1.addEvidenceTabFile( estream2, fg );
	typedef std::map<Var, size_t> Observation;
	vector<Observation> samples = e1.getEvidence();

	ParameterEstimation::loadDefaultRegistry();
	//	EMAlg::LOG_Z_TOL_DEFAULT = LIB_EM_TOLERANCE;
	//	EMAlg::MAX_ITERS_DEFAULT = EM_MAX_ITER;

	// assumes samples evenly divide
	size_t num_sample_iterations = (EM_MAX_SAMPLES-EM_INIT_SAMPLES)/EM_SAMPLES_DELTA + 1;
	stringstream *ss_data = new stringstream[num_sample_iterations*RANDOM_EM_TRIALS];

	// TODO: fixed params not local in this loop
	// TODO: keep samples identical between shared/non-shared
	//
	for (size_t i=EM_INIT_SAMPLES; i<=EM_MAX_SAMPLES; i*=EM_SAMPLES_DELTA) {
		cout << "samples: " << i << endl;
		vector<Observation> samples_copy = samples;
		//random_shuffle ( samples_copy.begin(), samples_copy.end() );
		samples_copy.resize(i);
#pragma omp parallel for \
		schedule(static, 1)
		for (size_t k=0; k<RANDOM_EM_TRIALS; k++) {
			stringstream maxiterss;
			stringstream sampledataout;
			//FactorGraph local_fg = *fg.clone();
			PropertySet infprops;
			infprops.set( "verbose", VERBOSE );
			infprops.set( "updates", string("HUGIN") );
			InfAlg* inf1 = newInfAlg( INF_TYPE, fg, infprops );
			if (init == 1)
				randomize_fg(&inf1->fg());
			else if (init == 3)
				noise_fg(&inf1->fg());

			//			stringstream trials_out;
			//			trials_out << "Iteration\tLikelihood\tL2_error" << endl;
			ss_data[(i-EM_INIT_SAMPLES)/EM_SAMPLES_DELTA*RANDOM_EM_TRIALS+k] << "Iteration\tLikelihood\tL2_error" << endl;

			inf1->init();
			Evidence e(samples_copy);

			stringstream emstream(emFile);
			EMAlg em(e, *inf1, emstream);
			em._log_z_tol = LIB_EM_TOLERANCE;
			em._max_iters = EM_MAX_ITER;

			Real l1;
			// Iterate EM until convergence
			while( !em.hasSatisfiedTermConditions() ) {
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
				double fg_diff = compareFG(&inf1->fg(), &fg_orig);
				//				trials_out << em.Iterations() << "\t" << l1 << "\t" <<
				//						fg_diff << endl;
				ss_data[(i-EM_INIT_SAMPLES)/EM_SAMPLES_DELTA*RANDOM_EM_TRIALS+k] << em.Iterations() << "\t" << l1 << "\t" <<
						fg_diff << endl;
			}
			double fg_diff = compareFG(&inf1->fg(), &fg_orig);
			//			sampledataout << e.nrSamples() << "\t" << l1 << "\t"
			//					<< em.Iterations() << "\t" << fg_diff << endl;

			delete inf1;

		}
		//#pragma omp critical
		//		{
		//			fout << sampledataout.str();
		//		}

	}

	cout << "writing data to files... " << trials_dir << endl;
	//fout.close();

	for (size_t i=EM_INIT_SAMPLES; i<=EM_MAX_SAMPLES; i*=EM_SAMPLES_DELTA) {
		string temp_s = trials_dir;
		temp_s = temp_s.append("n").append(convertInt(i)).append("/");
		system(string("mkdir -p ").append(temp_s).c_str());
		for (size_t k=0; k<RANDOM_EM_TRIALS; k++) {
			ofstream f_outl2;
			string tmp = temp_s;
			f_outl2.open(tmp.append(convertInt(k)).c_str());
			f_outl2 << ss_data[(i-EM_INIT_SAMPLES)/EM_SAMPLES_DELTA*RANDOM_EM_TRIALS+k].str() << endl;
			f_outl2.close();
		}
	}
	delete[] ss_data;
}

void compareEM(char* fgIn, char* emIn1, char* emIn2) {

	FactorGraph fg;
	fg.ReadFromFile( fgIn );
	cout << "Initial samples: " << EM_INIT_SAMPLES << endl;
	cout << "Max samples: " << EM_MAX_SAMPLES << ". Increase by " <<
			EM_SAMPLES_DELTA << endl;
	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set( "verbose", VERBOSE );
	infprops.set( "updates", string("HUGIN") );

	cout << "Creating file: shared_compare.dat" << endl;
	ofstream fout("shared_compare.dat");
	fout.precision(12);
	fout << "numSamples\tlikelihood1\titerations1\tlikelihood2\titerations2" << endl;
	for (int i=EM_INIT_SAMPLES; i<=EM_MAX_SAMPLES; i+=EM_SAMPLES_DELTA) {
		InfAlg* inf1 = newInfAlg( INF_TYPE, fg, infprops );
		inf1->init();
		InfAlg* inf2 = newInfAlg( INF_TYPE, fg, infprops );
		inf2->init();
		string tabIn = generateTab(fgIn, i, NULL);
		Evidence e;
		ifstream estream( tabIn.c_str() );
		ifstream emstream1( emIn1 );
		ifstream emstream2( emIn2 );
		e.addEvidenceTabFile( estream, fg );
		estream.close();
		fout << e.nrSamples() << "\t";

		// Read EM specification

		EMAlg em1(e, *inf1, emstream1);
		EMAlg em2(e, *inf2, emstream2);

		Real l1;
		// Iterate EM until convergence
		while( !em1.hasSatisfiedTermConditions() ) {
			l1 = em1.iterate();
			cout << "em1: Iteration " << em1.Iterations() << " likelihood: " << l1 
					<< " avg: " << l1/100*(i+1)<<endl;
		}

		Real l2;
		// Iterate EM until convergence
		while( !em2.hasSatisfiedTermConditions() ) {
			l2 = em2.iterate();
			cout << "em2: Iteration " << em2.Iterations() << " likelihood: " << l2 
					<< " avg: " << l1/100*(i+1) <<endl;
		}

		FactorGraph fg_non = inf1->fg();
		FactorGraph fg_shared = inf2->fg();

		double fg_diff_shared_from_true = compareFG(&fg_shared, &fg);
		double fg_diff_from_true = compareFG(&fg_non, &fg);
		double fg_diff_shared_from_non_shared = compareFG(&fg_non, &fg_shared);

		cout << "FG diff shared from true: " << fg_diff_shared_from_true << endl;
		cout << "FG diff non-shared from true: " << fg_diff_from_true << endl;
		cout << "FG diff shared from non-shared: " << fg_diff_shared_from_non_shared << endl;

		fout << l1 << "\t" << em1.Iterations() << "\t";
		fout << l2 << "\t" << em2.Iterations() << "\t";
		fout << fg_diff_shared_from_true << "\t" << fg_diff_from_true << "\t" <<
				fg_diff_shared_from_non_shared << endl;
		emstream1.close();
		emstream2.close();
		delete inf1;
		delete inf2;
	}
	// Clean up

	fout.close();

}


int main(int argc, char* argv[]) {
	clock_t t1 = clock();
	if (argc == 3) {
		if (strcmp(argv[1],"-dot") == 0)
			printDot(argv[2]);
		else {
			int argCheck = atoi(argv[2]);
			if (argCheck == 0)
				displayStats(argv);
			else
				generateTab(argv[1], argCheck, NULL);
		}
	}
	else if(argc == 2) {

		displayStats(argv);
		//generateTab(argv[1], 100);
	}

	else if (argc == 4) {
		// expecting .fg, .tab, .em 
		if (strcmp(argv[1],"-s") == 0)
			doEmSamples(argv[2], argv[3], 0, NULL);
		else
			doEm(argv[1], argv[2], argv[3], 0);
	}
	else if (argc >= 5) {
		// expecting -c, fg, em, em
		if (strcmp(argv[1],"-c") == 0) {
			compareEM(argv[2], argv[3], argv[4]);
		}
		else if (strcmp(argv[1],"-s") == 0) {
			string py_cmd = "NULL";
			if (argc >= 6) {
				stringstream py_cmd_str;
				for (int k=5; k<argc; k++)
					py_cmd_str << argv[k] << " ";
				py_cmd = py_cmd_str.str();
			}
			if (strcmp(argv[2],"-random") == 0)
				doEmSamples(argv[3], argv[4], 1, py_cmd);
			else if (strcmp(argv[2],"-uniform") == 0)
				doEmSamples(argv[3], argv[4], 2, py_cmd);
			else if (strcmp(argv[2],"-noise") == 0)
				doEmSamples(argv[3], argv[4], 3, py_cmd);
			else if (strcmp(argv[2],"-all") == 0) {
				doEmSamples(argv[3], argv[4], 0, py_cmd);
				doEmSamples(argv[3], argv[4], 1, py_cmd);
				doEmSamples(argv[3], argv[4], 2, py_cmd);
				doEmSamples(argv[3], argv[4], 3, py_cmd);
			}

		}
		// expecting -random fg tab em
		else if (strcmp(argv[1],"-random") == 0) {
			doEm(argv[2], argv[3], argv[4], 1);
		}
		else if (strcmp(argv[1],"-uniform") == 0) {
			doEm(argv[2], argv[3], argv[4], 2);
		}

		else if (strcmp(argv[1],"-noise") == 0) {
			doEm(argv[2], argv[3], argv[4], 3);
		}
		else if (strcmp(argv[1],"-all") == 0) {
			doEm(argv[2], argv[3], argv[4], 0);
			doEm(argv[2], argv[3], argv[4], 1);
			doEm(argv[2], argv[3], argv[4], 2);
			doEm(argv[2], argv[3], argv[4], 3);
#ifdef DO_RANDOM_EM
			doRandomEM(argv[2], argv[3], argv[4], RANDOM_EM_TRIALS, RANDOM_EM_ITERATIONS);
			//doRandomEM(argv[2], argv[3], argv[4], 500, 3);
#endif
		}
		else if (strcmp(argv[1],"-many_random") == 0)
			doRandomEM(argv[2], argv[3], argv[4], RANDOM_EM_TRIALS, RANDOM_EM_ITERATIONS);
		else {
			printUsage();
			return 0;
		}

	}
	else {
		printUsage();
		return 0;
	}

	clock_t t2 = clock();
	cout.precision(7);
	cout << endl << "elapsed time: " << difftime(t2,t1)/1e6/60.0 << " CPU minutes." << endl;

	return 0;
}



