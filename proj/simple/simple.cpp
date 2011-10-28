/*  This file is part of libDAI - http://www.libdai.org/
 *
 *  Copyright (c) 2006-2011, The libDAI authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
 */

//TODO: input .fg .tab output likelihood

#include <dai/alldai.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace dai;


void displayStats(char** argv) {


	// This example program illustrates how to learn the
	// parameters of a Bayesian network from a sample of
	// the sprinkler network discussed at
	// http://www.cs.ubc.ca/~murphyk/Bayes/bnintro.html
	//
	// The factor graph file (sprinkler.fg) has to be generated first
	// by running example_sprinkler, and the data sample file
	// (sprinkler.tab) by running example_sprinkler_gibbs

	// Read the factorgraph from the file
	FactorGraph SprinklerNetwork;
	SprinklerNetwork.ReadFromFile( argv[1] );

	// Read sample from file
	Evidence e;
	ifstream estream( argv[2]);
	e.addEvidenceTabFile( estream, SprinklerNetwork );
	cout << "Number of samples: " << e.nrSamples() << endl;


	// Iterate EM until convergence
	//    while( !em.hasSatisfiedTermConditions() ) {
	//        Real l = em.iterate();
	//        cout << "Iteration " << em.Iterations() << " likelihood: " << l <<endl;
	//    }

	// Output true factor graph
	cout << endl << "Given factor graph:" << endl << "##################" << endl;
	cout.precision(12);
	cout << SprinklerNetwork;

	cout << SprinklerNetwork.nrVars() << " variables" << endl;
	cout << SprinklerNetwork.nrFactors() << " factors" << endl;


	// Calculate joint probability of all four variables
	Factor P;
	for( size_t I = 0; I < SprinklerNetwork.nrFactors(); I++ )
		P *= SprinklerNetwork.factor( I );
	// P.normalize();  // Not necessary: a Bayesian network is already normalized by definition

	// Calculate some probabilities
	Real denom = P.marginal( SprinklerNetwork.var(0) )[0];
	cout << "P([node 0]=1) = " << denom << endl;

	//cout << "P(S=1 | W=1) = " << P.marginal( VarSet( S, W ) )[3] / denom << endl;
	//cout << "P(R=1 | W=1) = " << P.marginal( VarSet( R, W ) )[3] / denom << endl;

}

void generateTab(char* input, int numSamples) {
	string outName = input;

	string::size_type pos = 0;
	pos = outName.find(".fg", pos);
	if (pos != string::npos)
		outName.replace(pos, outName.size(), ".tab");
	else
		outName.append(".tab");

	cout << "Generating " << outName << endl;
	cout << "Number of samples: " << numSamples << endl;
	if (numSamples < 1)
		throw;
	FactorGraph factorIn;
	factorIn.ReadFromFile( input );
	cout << "Sprinkler network read from sprinkler.fg" << endl;

	// Output some information about the factorgraph
	cout << factorIn.nrVars() << " variables" << endl;
	cout << factorIn.nrFactors() << " factors" << endl;

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

	// Draw samples from joint distribution using Gibbs sampling
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

}

int main(int argc, char* argv[]) {

	if (argc == 3) {
		cout << "Displaying fg/tab stats" << endl;
		int argCheck = atoi(argv[2]);
		if (argCheck == 0)
			displayStats(argv);
		else
			generateTab(argv[1], argCheck);

	}
	else if(argc == 2) {

		generateTab(argv[1], 100);
	}
	else {
		//cout << "Not enough args [" << argc-1 <<"]. Requires 2: an .fg file and .tab file." << endl;
		cout << "--- Usage ---" << endl;

		cout << "display fg/tab stats:\n\tsimple asd.fg asd2.tab" << endl;
		cout << "generate tab file (output will be *.tab): \n\tsimple asd.fg [num_samples]" << endl;
	}



	return 0;
}
