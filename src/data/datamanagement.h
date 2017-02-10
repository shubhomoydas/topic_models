/*
 * datamanagement.h
 *
 *  Created on: Oct 26, 2013
 *      Author: Moy
 */

#ifndef DATAMANAGEMENT_H_
#define DATAMANAGEMENT_H_

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "../common/utils.h"

namespace osu {

	namespace data {

	osu::data::SparseInputData *loadSparseVectors(std::string& filename,
			std::string& vocabfile, int nholdout = 0);

	void populateResultsMapFromStringVector(
		const std::vector< std::vector<std::string>* >& recs,
		std::map<std::string,int> *results);
	void loadResults(const std::string& filename,
		std::map<std::string,int> *results, const int skiptoplines);

	template<typename T>
	osu::data::DataMat<T>* populateMatFromStringVectorOfNumeric(
			const std::vector< std::vector<std::string>* >& recs, const int skipleftcols,
			const int skiprightcols) {
		int cols = 0;
		if (recs.size() > 0) {
			cols = recs[0]->size();
		}
		osu::data::DataMat<T> *mat =
				new DataMat<T>(recs.size(),
						cols-skipleftcols-skiprightcols);
		int i = 0;
		for ( std::vector< std::vector<std::string>* >::const_iterator it=recs.begin() ;
				it < recs.end(); it++, i++ ) {
			std::vector<std::string>* tokens = *it;
			int j = 0;
			for ( std::vector<std::string>::iterator itt=tokens->begin() ;
					itt < tokens->end(); itt++, j++ ) {
				if (j >= cols-skiprightcols) {
					break;
				}
				if (j >= skipleftcols) {
					mat->data[i][j-skipleftcols] = (T)atof((*itt).c_str());
				}
			}
			//std::cout << "Line " << i << std::endl;
		}
		return mat;
	}
	std::vector<std::string>* loadCSVCol(const std::string& filename,
				const int skiptoplines, const int startCol, const int nCols=1);
	template<typename T>
	DataMat<T>* loadCSV(const std::string& filename, const bool skipheader, const int skipleftcols, const int skiprightcols) {
		return loadCSV<T>(filename, (skipheader ? 1 : 0), skipleftcols, skiprightcols);
	}
	template<typename T>
	DataMat<T>* loadCSV(const std::string& filename, const int skiptoplines,
			const int skipleftcols, const int skiprightcols) {
		DataMat<T>* mat = NULL;
		std::ifstream ifile (filename.c_str());
		int skip = skiptoplines;
		if (ifile.is_open()) {
			std::vector<std::vector<std::string>* >* recs =
					new std::vector< std::vector<std::string>* >;
			int reccount = 0;
			//std::cout << "Opened the file..." << std::endl;
			while ( ifile.good() ) {
				std::string line;
				getline (ifile,line);
				if (skip > 0) {
					skip--;
					continue;
				}
				osu::trim(line);
				//std::cout << line << std::endl;
				if (line.length() == 0) continue;
				reccount++;
				std::vector<std::string>* tokens = new std::vector<std::string>;
				osu::tokenize(line, *tokens, ",");
				recs->push_back(tokens);
				if (reccount == 1) {
					// get the number of columns
					std::cout << "Columns: " << tokens->size() << "; line length: " << line.length() << std::endl;
				}
			}
			ifile.close();
			std::cout << "Total rows read: " << reccount << std::endl;
			mat = populateMatFromStringVectorOfNumeric<T>(*recs, skipleftcols, skiprightcols);
			std::cout << "Populated matrix: " << std::endl;
			osu::releaseVectorElements< std::vector<std::string> >(recs);
			std::cout << "Released data: " << std::endl;
			delete recs;
		} else std::cout << "Unable to open file " << filename << std::endl;
		return mat;
	}

	DataMat<int>* loadIntCol(const std::string& filename,
				const int skiptoplines, const int col);

	template<class T>
	void saveDataToCSV(const T **data, int rows, int cols,
			bool header, const std::string& filepath) {
		if (filepath.empty()) {
			std::cerr << "Output filename missing..." << std::endl;
			return;
		}
		std::ofstream outfile(filepath.c_str());
		if (header) {
			for (int i = 0; i < cols; i++) {
				if (i > 0) outfile << ",";
				outfile << "col_" << i;
			}
		}
		outfile << std::endl;
		for (int i = 0; i < rows; i++) {
			T *rowData = data[i];
			for (int j = 0; j < cols; j++) {
				if (j > 0) outfile << ",";
				outfile << (T)(rowData[j]);
			}
			outfile << std::endl;
		}
		outfile.close();
	}

	} // data

} // osu


#endif /* DATAMANAGEMENT_H_ */
