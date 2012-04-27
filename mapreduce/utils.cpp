// erik reed
// erikreed@cmu.edu
#include "dai_mapreduce.h"

int main(int argc, char* argv[]) {
	if (argc == 1)
		return -1;
	if (argc == 2) {
		string flag = argv[1];
		if (flag == "-u"){
			// get post mapreduce data -- update
			string s;
			while (std::getline(std::cin, s)) {
				str_char_replace(s,'^','\n');
				EMdata dat = stringToEM(s);

				ostringstream outname;
				outname << "out/dat." << dat.bnID;
				ofstream fout;
				fout.open (outname.str().c_str());
				fout << s << endl;
				fout.close();

//				outname.clear();
//				outname << "in/fg." << dat.bnID;
//				fout.open (outname.str().c_str());
//				fout << dat.fgFile << endl;
//				fout.close();

				cout << "iter: " << dat.iter << "\t likelihood: " << dat.likelihood << endl;
			}
		}
		// create initial serialized files with iter=0
		if (flag == "-c") {


		}
		return 0;
	}

	rnd_seed(time(NULL));
	FactorGraph fg;
	fg.ReadFromFile(argv[1]);

	string emFile = readFile("in/em");

	for (size_t i=2; i<argc; i++) {
		randomize_fg(&fg);

		EMdata datForMapper;
		datForMapper.iter = 0;
//		datForMapper.emFile = emFile;
		datForMapper.likelihood = 0;
		datForMapper.bnID = -1;

		ostringstream ss;
		ss << fg;
		datForMapper.fgFile = ss.str();

		ofstream fout;
		fout.open (argv[i]);
		fout << emToString(datForMapper) << endl;
		fout.close();
	}

}
