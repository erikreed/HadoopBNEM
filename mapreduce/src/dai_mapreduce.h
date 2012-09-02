/*
 * dai_mapreduce.h
 *
 *  Created on: Apr 24, 2012
 *      Author: erik reed
 */

#ifndef DAI_MAPREDUCE_H_
#define DAI_MAPREDUCE_H_

#include <dai/alldai.h>
#include <src/base64.cpp>

#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

using namespace std;
using namespace dai;

const string INF_TYPE = "JTREE";
const Real LIB_EM_TOLERANCE = 1e-3;
const size_t EM_MAX_ITER = 100;

// ALEM (Saluja et al) parameters
const size_t pop_size = 250; // i.e. EMruns, denoted N
const size_t numLayers = 5;
const double agegap = 5; // denoted a
const bool verbose = true;
const size_t min_runs_layer0 = 5;
const size_t min_runs_intermediate = 2;
// end ALEM parameters

struct EMdata {
	string emFile;
	FactorGraph fg;
	string tabFile;
	size_t iter;
	Real likelihood;
	Real lastLikelihood;
	vector<MaximizationStep> msteps;
	int bnID;
	int ALEM_layer;


	// TODO: optimize serialization
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & emFile;
		ar & fg;
		ar & tabFile;
		ar & iter;
		ar & likelihood;
		ar & lastLikelihood;
		ar & msteps;
		ar & bnID;
		ar & ALEM_layer;
	}

	bool isConverged() {
	    if( iter >= EM_MAX_ITER )
	        return true;
	    else if( iter < 3 )
	        // need at least 2 to calculate ratio
	        // Also, throw away first iteration, as the parameters may not
	        // have been normalized according to the estimation method
	        return false;
	    else {
	        if( lastLikelihood == 0 )
	            return false;
	        Real diff = likelihood - lastLikelihood;
	        if( diff < 0 ) {
	            cerr << "Error: in EM log-likehood decreased from " << lastLikelihood << " to " << likelihood << endl;
	            return true;
	        }
	        return (diff / fabs(lastLikelihood)) <= LIB_EM_TOLERANCE;
	    }
	}
};

int getNumRuns(vector<vector<EMAlg*> > &emAlgs) {
	int sum = 0;
	foreach(vector<EMAlg*> &layer, emAlgs) {
		foreach(EMAlg* em, layer) {
			if (!em->hasSatisfiedTermConditions())
				sum++;
		}
	}
	return sum;
}

int getNumRuns(vector<vector<EMdata> > &emAlgs) {
	int sum = 0;

	foreach(vector<EMdata> &layer, emAlgs) {
		foreach(EMdata &em, layer) {
			if (!em.isConverged())
				sum++;
		}
	}
	return sum;
}

string emToString(const EMdata &em) {
	ostringstream s(ios::binary);
	boost::archive::binary_oarchive oa(s);
	oa << em;
	const string nonEncoded = s.str();
	return base64_encode(reinterpret_cast<const unsigned char*>(nonEncoded.c_str()),
			nonEncoded.size());
}

EMdata stringToEM(const string &s) {
	string decoded = base64_decode(s);
	istringstream ss(decoded, ios::binary);
	boost::archive::binary_iarchive ia(ss);
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

void str_char_replace(string &s, char from, char to) {
	replace( s.begin(), s.end(), from, to);
}

bool str_replace(string& str, const string& from,
		const string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == string::npos)
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

vector<string> &str_split(const string &s, char delim,
		vector<string> &elems) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim))
		elems.push_back(item);
	return elems;
}

vector<string> str_split(const string &s, char delim) {
	vector<string> elems;
	return str_split(s, delim, elems);
}
PropertySet getProps() {
	PropertySet infprops;
	infprops.set("verbose", (size_t) 0);
	infprops.set("updates", string("HUGIN"));
	infprops.set("log_z_tol", LIB_EM_TOLERANCE);
	infprops.set("MAX_ITERS", EM_MAX_ITER);
	return infprops;
}

#endif /* DAI_MAPREDUCE_H_ */
