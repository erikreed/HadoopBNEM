// erik reed
// erikreed@cmu.edu

#include <dai/alldai.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <fstream>

using namespace std;
using namespace dai;

#define INTERMEDIATE_VALUES
#define NOISE_AMOUNT .05 // corresponding to 5%
#define INF_TYPE "JTREE"

// constants for compareEM(...)
#define EM_MAX_SAMPLES 1000
#define EM_SAMPLES_DELTA 25
#define EM_INIT_SAMPLES 50

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

string generateTab(char* input, int numSamples) {
	string outName = input;

	string::size_type pos = 0;
	pos = outName.find(".fg", pos);
	if (pos != string::npos)
		outName.replace(pos, outName.size(), ".tab");
	else
		outName.append(".tab");

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
	gibbsProps.set("verbose", size_t(0));
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

void randomize_fg(FactorGraph* fg) {
	srand((unsigned)time(NULL));
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
		f.normalize(NORMPROB);
		fg->setFactor(i, f, false);
	}
}

void doEm(char* fgIn, char* tabIn, char* emIn, int init) {
	FactorGraph fg;
	fg.ReadFromFile( fgIn );

	srand((unsigned)time(NULL));

	string outname;

	if (init == 1) {
		// random
		cout << "Using random initialization" << endl;
		randomize_fg(&fg);
		outname = "output.dat.random";
	}
	else if (init == 2) {
		// uniform
		cout << "Using uniform initialization" << endl;
		uniformize_fg(&fg);
		outname = "output.dat.uniform";
	}
	else if (init == 3) {
		// noise
		cout << "Using noisy initialization. Noise value: +/- " <<
				NOISE_AMOUNT*100 << "%" << endl;
		noise_fg(&fg);
		outname = "output.dat.noise";
	}
	else {
		cout << "Using given .fg for initialization" << endl;
		outname = "output.dat.default";
	}
	cout << "Writing results to: " << outname.c_str() << endl;
	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set( "verbose", (size_t)1 );
	infprops.set( "updates", string("HUGIN") );
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

	cout.precision(16);

	stringstream s_out;
	//s_out << "[\n";
	s_out.precision(16);

	for (size_t i=0; i< fg.nrFactors(); i++) {
		s_out << fg.factor(i).nrStates();
		if (i != fg.nrFactors() - 1)
			s_out << ", ";
	}
	s_out << endl;
	// initial values
	printEMIntermediates(&s_out, inf);
	s_out << endl;
	// Iterate EM until convergence
	while( !em.hasSatisfiedTermConditions() ) {

		Real l = em.iterate();
		cout << "Iteration " << em.Iterations() << " likelihood: " << l <<endl;

		printEMIntermediates(&s_out, inf);
		s_out << endl;
		s_out.flush();
	}
	//s_out << "]";
/*
	// Output true factor graph
	cout << endl << "True factor graph:" << endl << "##################" << endl;
	cout.precision(12);

	cout << fg;

	// Output learned factor graph
	cout << endl << "Learned factor graph:" << endl << "#####################" << endl;
	cout.precision(12);
	cout << inf->fg();
*/
	// Clean up
	delete inf;
	cout << "Intermediate values: an array of tab delimited K*N " <<
			"where N=numFactors and K=iterations." << endl;
	cout << "Includes initial values (i.e. iteration 0)" << endl;

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
	
	cout << endl << "--- Info ---" << endl;
	cout << "Results output to \"output.dat\"" << endl;
	cout << "Builtin inference algorithms: " << builtinInfAlgNames() << endl;
	cout << "Currently used inference algorithm: " << INF_TYPE << endl;
	//cout << "compare FG files (e.g. checking difference from true vs. EM generated FGs)\n\t" << 
	//	"./simple -f asd.fg asd2.fg" << endl;
}

double compareFG(FactorGraph* fg1, FactorGraph* fg2) {
	size_t numFactors = fg1->nrFactors();
	if (fg2->nrFactors() != numFactors)
		throw;
	double sum=0;
	for (size_t i=0; i< numFactors; i++) {
		Factor f1 = fg1->factor(i);
		Factor f2 = fg2->factor(i);
		cout << "sum: " << (f1-f2).sumAbs() << endl;
		sum += abs((f1-f2).sumAbs());
	}
	//sum = sqrt(sum);

	return sum;
}

void compareEM(char* fgIn, char* emIn1, char* emIn2) {

	FactorGraph fg;
	fg.ReadFromFile( fgIn );
	cout << "Initial samples: " << EM_INIT_SAMPLES << endl;
	cout << "Max samples: " << EM_MAX_SAMPLES << ". Increase by " <<
		EM_SAMPLES_DELTA << endl;
	// Prepare junction-tree object for doing exact inference for E-step
	PropertySet infprops;
	infprops.set( "verbose", (size_t)1 );
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
		string tabIn = generateTab(fgIn, i);
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
		int argCheck = atoi(argv[2]);
		if (argCheck == 0)
			displayStats(argv);
		else
			generateTab(argv[1], argCheck);

	}
	else if(argc == 2) {
		displayStats(argv);
		//generateTab(argv[1], 100);
	}
	else if (argc == 4) {
		// expecting .fg, .tab, .em 
		doEm(argv[1], argv[2], argv[3], 0);
	}
	else if (argc == 5) {
		// expecting -c, fg, em, em
		if (strcmp(argv[1],"-c") == 0) {
			compareEM(argv[2], argv[3], argv[4]);
		}
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
			doEm(argv[2], argv[3], argv[4], 1);
			doEm(argv[2], argv[3], argv[4], 2);
			doEm(argv[2], argv[3], argv[4], 3);
		}
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
	cout << endl << "elapsed time: " << difftime(t2,t1)/1e6 << " seconds" << endl;

	return 0;
}



