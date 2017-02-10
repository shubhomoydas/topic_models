/*
 * DataArchiver.h
 *
 *  Created on: Oct 31, 2013
 *      Author: Moy
 */

#ifndef DATAARCHIVER_H_
#define DATAARCHIVER_H_

#include<fstream>
#include "./Options.h"
#include "../common/utils.h"

namespace osu {

class DataArchiver {
private:
	std::ofstream *thetasfile;
	std::ofstream *tausfile;
	std::ofstream *pisfile;
	std::ofstream *gammasfile;
	std::ofstream *groupsfile;
public:
	DataArchiver();
	DataArchiver(Options& options):
		thetasfile(0), tausfile(0), pisfile(0), gammasfile(0), groupsfile(0) {
		initialize(options);
	}

	bool outputThetas() { return thetasfile != 0; }
	bool outputTaus() { return tausfile != 0; }
	bool outputPis() { return pisfile != 0; }
	bool outputGammas() { return gammasfile != 0; }
	bool outputGroups() { return groupsfile != 0; }

	void initialize(Options& options);

	template<class T>
	void add(T** data, int rows, int cols, int type, int idx) {
		switch (type) {
		case 0:
			// output theta
			if (thetasfile)
				osu::output_data(*thetasfile, data, rows, cols, idx);
			//else
			//	osu::output_data(std::cout, data, rows, cols, idx);
			break;
		case 1:
			// output tau
			if (tausfile)
				osu::output_data(*tausfile, data, rows, cols, idx);
			//else
			//	osu::output_data(std::cout, data, rows, cols, idx);
			break;
		case 2:
			// output tau
			if (pisfile)
				osu::output_data(*pisfile, data, rows, cols, idx);
			//else
			//	osu::output_data(std::cout, data, rows, cols, idx);
			break;
		case 3:
			// output groups
			if (groupsfile)
				osu::output_data(*groupsfile, data, rows, cols, idx);
				//osu::output_data(*groupsfile, data[0], rows, idx);
			//else
			//	osu::output_data(std::cout, data, rows, cols, idx);
			break;
		case 4:
			// output groups
			if (gammasfile)
				osu::output_data(*gammasfile, data, rows, cols, idx);
				//osu::output_data(*groupsfile, data[0], rows, idx);
			//else
			//	osu::output_data(std::cout, data, rows, cols, idx);
			break;
		default:
			break;
		}
	}
	void close();
	virtual ~DataArchiver();
};

}
#endif /* DATAARCHIVER_H_ */
