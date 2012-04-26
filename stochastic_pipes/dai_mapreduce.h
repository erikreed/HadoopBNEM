/*
 * dai_mapreduce.h
 *
 *  Created on: Apr 24, 2012
 *      Author: erik reed
 */

#ifndef DAI_MAPREDUCE_H_
#define DAI_MAPREDUCE_H_

#include <dai/alldai.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

using namespace std;
using namespace dai;

#define INF_TYPE "JTREE"

const Real LIB_EM_TOLERANCE = 1e-9;
const size_t EM_MAX_ITER = 100;

struct EMdata {
	string emFile;
	string fgFile;
	string tabFile;
	size_t iter;
	Real likelihood;
	vector<MaximizationStep> msteps;
	int bnID;


	// TODO: optimize serialization
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

inline string emToString(const EMdata &em) {
	ostringstream s;
	s << std::scientific;
	boost::archive::text_oarchive oa(s);
	oa << em;
	return s.str();
}

inline EMdata stringToEM(const string &s) {
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

void str_char_replace(string &s, char from, char to) {
	replace( s.begin(), s.end(), from, to);
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

#endif /* DAI_MAPREDUCE_H_ */
