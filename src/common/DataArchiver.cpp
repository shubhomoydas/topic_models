/*
 * DataArchiver.cpp
 *
 *  Created on: Oct 31, 2013
 *      Author: Moy
 */

#include <fstream>
#include "./Options.h"

#include "DataArchiver.h"

namespace osu {

DataArchiver::DataArchiver():
		thetasfile(0), tausfile(0), pisfile(0), gammasfile(0), groupsfile(0) {
}

void DataArchiver::initialize(osu::Options& options) {
	close();
	if (!options.thetas_file.empty())
		thetasfile = new std::ofstream(
				options.thetas_file.c_str() ,
				std::ios::out);
	if (!options.taus_file.empty())
		tausfile = new std::ofstream(
				options.taus_file.c_str() ,
				std::ios::out);
	if (!options.pis_file.empty())
		pisfile = new std::ofstream(
				options.pis_file.c_str() ,
				std::ios::out);
	if (!options.gammas_file.empty())
		gammasfile = new std::ofstream(
				options.gammas_file.c_str() ,
				std::ios::out);
	if (!options.groups_file.empty())
		groupsfile = new std::ofstream(
				options.groups_file.c_str() ,
				std::ios::out);
}

void DataArchiver::close() {
	if (thetasfile) {
		if (thetasfile->is_open()) thetasfile->close();
		delete thetasfile;
		thetasfile = 0;
	}
	if (tausfile) {
		if (tausfile->is_open()) tausfile->close();
		delete tausfile;
		tausfile = 0;
	}
	if (pisfile) {
		if (pisfile->is_open()) pisfile->close();
		delete pisfile;
		pisfile = 0;
	}
	if (gammasfile) {
		if (gammasfile->is_open()) gammasfile->close();
		delete gammasfile;
		gammasfile = 0;
	}
	if (groupsfile) {
		if (groupsfile->is_open()) groupsfile->close();
		delete groupsfile;
		groupsfile = 0;
	}
}

DataArchiver::~DataArchiver() {
	close();
}

}
