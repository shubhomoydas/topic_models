/*
 * csvdata.cpp
 *
 *  Created on: Oct 26, 2013
 *      Author: Moy
 */

#include "../common/utils.h"
#include "datamanagement.h"
#include "../math/math.h"

namespace osu {

	namespace data {

	osu::data::DataMat<int> *vector2mat(std::vector<int>& vec) {
		int sz = vec.size();
		osu::data::DataMat<int> *mat = new osu::data::DataMat<int>(1,sz);
		int **data = mat->data;
		for (int i = 0; i < sz; i++) {
			data[0][i] = vec.at(i);
		}
		return mat;
	}

	osu::data::SparseInputData *loadSparseVectors(std::string& filename,
			std::string& vocabfile, int nholdout) {
		if (filename.empty()) {
			std::cout << "WARN:: Input filename not provided!!" << std::endl;
			return 0;
		}
		if (vocabfile.empty()) {
			std::cout << "WARN:: Vocab filename not provided!!" << std::endl;
			return 0;
		}
		std::ifstream ifile (filename.c_str());
		DataMat<int>* featureidxs = loadIntCol(vocabfile, 0, 0);
		int vocabSize = featureidxs->cols;
		int maxfeatureidx = osu::max(featureidxs->data[0], vocabSize);
		if (maxfeatureidx+1 != vocabSize) {
			std::cout << "WARN:: Vocab size is inconsistent!! Max+1 (" <<
					maxfeatureidx << "+1) must equal " << vocabSize << std::endl;
		}
		std::vector<int> userids;
		std::map<int,int> *groupids = new std::map<int,int>();
		std::vector<osu::data::SparseVec<int>*> *instances =
				new std::vector<osu::data::SparseVec<int>*>();
		int skip = 0; // in case we need to skip lines from top
		if (ifile.is_open()) {
			int reccount = 0;
			//cout << "Opened the file..." << endl;
			while ( ifile.good() ) {
				std::string line;
				getline (ifile,line);
				if (skip > 0) {
					skip--;
					continue;
				}
				trim(line);
				if (line.length() == 0) continue;
				reccount++;
				std::vector<std::string>* tokens = new std::vector<std::string>;
				tokenize(line, *tokens, ",");
				int ntokens = tokens->size();
				if (ntokens < 2) {
					std::cout << "WARN:: Invalid input pattern at line " << reccount << std::endl;
					continue;
				}
				int userid = atoi(tokens->at(1).c_str());
				int groupid = atoi(tokens->at(0).c_str());
				groupids->insert(std::pair<int,int>(userid,groupid));
				userids.push_back(userid);
				int *idxs = new int[ntokens-2];
				int *vals = new int[ntokens-2];
				for (int i = 2; i < ntokens; i++) {
					std::string token = tokens->at(i);
					trim(token);
					std::string::size_type pos = token.find(':',0);
					if (pos != std::string::npos) {
						idxs[i-2] = atoi(token.substr(0,pos).c_str());
						vals[i-2] = atoi(token.substr(pos+1).c_str());
					}
				}
				osu::data::SparseVec<int>* vec =
						new osu::data::SparseVec<int>(idxs, vals, ntokens-2, vocabSize);
				instances->push_back(vec);
				if (reccount == 1) {
					// get the number of columns
					//cout << "Columns: " << tokens->size() << "; line length: " << line.length() << endl;
				}
			}
			ifile.close();
		} else std::cout << "Unable to open file " << filename << std::endl;
		std::vector<osu::data::SparseVec<int>*> *trainInstances = instances;
		std::vector<osu::data::SparseVec<int>*> *holdoutInstances = 0;
		if (nholdout > 0) {
			int *rndindexes = osu::generate_random_without_replacement(nholdout, 0, instances->size()-1, true);
			//int *rndindexes = osu::generate_random_without_replacement(nholdout, 0, 10-1, true);
			if (rndindexes) {
				trainInstances = new std::vector<osu::data::SparseVec<int>*>();
				holdoutInstances = new std::vector<osu::data::SparseVec<int>*>();
				int n = instances->size();
				int p = 0;
				for (int i = 0; i < n; i++) {
					if (p < nholdout && i == rndindexes[p]) {
						holdoutInstances->push_back(instances->at(i));
						p++;
					} else {
						trainInstances->push_back(instances->at(i));
					}
				}
				// delete the instances vector WITHOUT deleting the contents
				delete instances;
			} else {
				std::cout << "WARN:: No holdout set created..." << std::endl;
				trainInstances = instances;
			}
			delete[] rndindexes;
		}
		return new osu::data::SparseInputData(
				trainInstances, vocabSize, groupids,
				vector2mat(userids), holdoutInstances
			);
	}

	// Each vector of a vector of strings represents one line in the results file.
	// The first token in each line is the rank and second is the id
	void populateResultsMapFromStringVector(const std::vector< std::vector<std::string>* >& recs, \
			std::map<std::string,int> *results) {
		for ( std::vector< std::vector<std::string>* >::const_iterator it=recs.begin() ; it < recs.end(); it++ ) {
			std::vector<std::string>* tokens = *it;
			if (tokens->size() > 1) {
				results->insert(std::pair<std::string, int>(tokens->at(1), atoi(tokens->at(0).c_str())));
			}
		}
	}

	void loadResults(const std::string& filename,
			std::map<std::string,int> *results, const int skiptoplines) {
		std::ifstream ifile (filename.c_str());
		int skip = skiptoplines;
		if (ifile.is_open()) {
			std::vector<std::vector<std::string>* >* recs =
					new std::vector< std::vector<std::string>* >;
			int reccount = 0;
			//cout << "Opened the file..." << endl;
			while ( ifile.good() ) {
				std::string line;
				getline (ifile,line);
				if (skip > 0) {
					skip--;
					continue;
				}
				trim(line);
				if (line.length() == 0) continue;
				reccount++;
				std::vector<std::string>* tokens = new std::vector<std::string>;
				tokenize(line, *tokens, ",");
				recs->push_back(tokens);
				if (reccount == 1) {
					// get the number of columns
					//cout << "Columns: " << tokens->size() << "; line length: " << line.length() << endl;
				}
			}
			ifile.close();
			//cout << "Total rows read: " << reccount << endl;
			populateResultsMapFromStringVector(*recs, results);
			osu::releaseVectorElements< std::vector<std::string> >(recs);
			delete recs;
		} else std::cout << "Unable to open file " << filename << std::endl;
	}

	std::vector<std::string>* loadCSVCol(const std::string& filename,
			const int skiptoplines, const int startCol, const int nCols) {
		std::vector<std::string> *strlist = new std::vector<std::string>();
		std::ifstream ifile (filename.c_str());
		int skip = skiptoplines;
		if (ifile.is_open()) {
			int reccount = 0;
			//cout << "Opened the file..." << endl;
			while ( ifile.good() ) {
				std::string line;
				getline (ifile,line);
				if (skip > 0) {
					skip--;
					continue;
				}
				trim(line);
				if (line.length() == 0) continue;
				reccount++;
				std::vector<std::string> tokens;
				tokenize(line, tokens, ",");
				if (nCols == 1)
					strlist->push_back(tokens.at(startCol));
				else {
					std::string acc;
					for (int i = 0; i < nCols && (startCol+i) < (int)tokens.size(); i++) {
						if (i > 0) acc.append(",");
						acc.append(tokens.at(startCol+i));
					}
					strlist->push_back(acc);
				}
				if (reccount == 1) {
					// get the number of columns
					//cout << "Columns: " << tokens.size() << "; line length: " << line.length() << endl;
				}
			}
			ifile.close();
			//cout << "Total rows read: " << reccount << endl;
		} else std::cout << "Unable to open file " << filename << std::endl;
		return strlist;

	}

	DataMat<int>* loadIntCol(const std::string& filename,
			const int skiptoplines, const int col) {
		std::vector<std::string>* csv = loadCSVCol(filename, skiptoplines, col);
		DataMat<int>* colData = new DataMat<int>(1, csv->size());
		int i = 0;
		std::vector<std::string>::iterator it;
		for (it = csv->begin(); it != csv->end(); it++, i++) {
			colData->data[0][i] = atoi((*it).c_str());
		}
		delete csv;
		return colData;
	}

	std::set<std::string>* loadIds(const std::string& filename, const int skiptoplines, const int col) {
		std::vector<std::string>* csv = loadCSVCol(filename, skiptoplines, col);
		std::set<std::string>* ids = new std::set<std::string>();
		std::vector<std::string>::iterator it;
		for (it = csv->begin(); it != csv->end(); it++) {
			ids->insert(*it);
		}
		delete csv;
		return ids;
	}

} // data

} // osu
