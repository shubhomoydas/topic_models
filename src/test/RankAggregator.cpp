/*
 * RankAggregator.cpp
 *
 *  Created on: Jan 17, 2014
 *      Author: Moy
 */

#include<fstream>
#include <utility>
#include <vector>
#include <cmath>
#include "RankAggregator.h"
#include "../data/datamanagement.h"
#include "../math/math.h"
#include "../topicmodel/simpleLDA.h"
#include "../topicmodel/multinomialNBLDA.h"

namespace osu {

void RankAggregator::normalizeRanks(osu::data::DataMat<int> *mat, osu::Options &options) {
	double N = mat->cols;
	double nlim = ((double)N)*options.cutoff_fraction; //std::min(N/2,(double)5000);
	double r = (1./nlim)*std::log((double)options.max_votes);
	for (int i = 0; i < mat->rows; i++) {
		int *src = mat->data[i];
		for (int j = 0; j < mat->cols; j++, src++) {
			*src = std::floor(((double)options.max_votes)*
					std::exp(-r*(((double)*src))));
		}
	}
}

std::vector< std::pair<int, double>* > *applyMaxConsensus(
		osu::data::DataMat<double> *thetas,
		osu::data::DataMat<double> *taus) {

	std::vector< std::pair<int, double>* > *consensusScores =
			new std::vector< std::pair<int, double>* >();

	for (int i = 0; i < taus->rows; i++) {
		double maxprob = 0;
		for (int j = 0; j < taus->cols; j++) {
			if (taus->data[i][j] > maxprob)
				maxprob = taus->data[i][j];
		}
		consensusScores->push_back(new std::pair<int, double>(i+1,maxprob));
	}

	return consensusScores;
}

std::vector< std::pair<int, double>* > *applyMeanConsensus(
		osu::data::DataMat<double> *thetas,
		osu::data::DataMat<double> *taus) {

	double thetasum[thetas->rows];
	std::vector< std::pair<int, double>* > *consensusScores =
			new std::vector< std::pair<int, double>* >();

	osu::setVal<double>(thetasum, thetas->rows, 0);
	osu::sum_cols(thetas->data, thetas->rows, thetas->cols, thetasum);

	for (int i = 0; i < taus->rows; i++) {
		double sumscores = 0;
		for (int j = 0; j < taus->cols; j++) {
			sumscores = sumscores + taus->data[i][j]*thetasum[j];
		}
		consensusScores->push_back(new std::pair<int, double>(i+1,sumscores));
	}

	return consensusScores;
}

std::vector< std::pair<int, double>* > *RankAggregator::applyConsensus(
		osu::data::DataMat<double> *thetas,
		osu::data::DataMat<double> *taus) {

	if (options.consensus_type == osu::ConsensusType::MEAN) {
		return applyMeanConsensus(thetas, taus);
	} else {
		return applyMaxConsensus(thetas, taus);
	}

}

void output_to_file(std::vector< std::pair<int, double>* > *data,
		std::vector<std::string> *pre, std::ofstream& out) {
	std::vector< std::pair<int, double>* >::iterator it = data->begin();
	for (int i = 0; it != data->end(); i++, it++) {
		if (pre) out << pre->at(i) << ",";
		//out << (*it)->first << "," << (*it)->second << std::endl;
		out << (*it)->second << std::endl;
	}
}

void RankAggregator::rank() {
	if (options.consensus_type == osu::ConsensusType::SIMPLE)
		rankSimple();
	else
		rankLDA();
}

void RankAggregator::rankSimple() {

	std::vector< std::pair<int, double>* > *consensusScores =
			new std::vector< std::pair<int, double>* >();
	std::cout << "rows: " << mat->rows << ", cols: " << mat->cols << std::endl;
	for (int i = 0; i < mat->cols; i++) {
		double sum = 0;
		for (int j = 0; j < mat->rows; j++) {
			sum = sum + mat->data[j][i];
		}
		consensusScores->push_back(new std::pair<int,double>(i, (double)sum/(double)mat->cols));
	}
	if (options.debug) std::cout << "Completed simple averaged consensus of normalized ranks ..." << std::endl;

	if (!options.consensus_file.empty()) {
		std::ofstream consensusfile(
				options.consensus_file.c_str() ,
				std::ios::out);
		for (int i = 0; i < options.prefix_cols; i++)
			consensusfile << "col_" << (i+1) << ",";
		consensusfile << "score" << std::endl;
		output_to_file(consensusScores, firstCols, consensusfile);
	}

	osu::releaseVectorElements< std::pair<int, double> >(consensusScores);
	delete consensusScores;
}

void RankAggregator::rankLDA() {

	std::vector<osu::data::SparseVec<int>*> *instances = osu::lda::convert_to_sparse(mat, options);
	if (options.debug) std::cout << "Initialized sparse vecs ..." << std::endl;

	osu::lda::SimpleLDA* lda = new osu::lda::SimpleLDA(instances, 0, 0, options);
	lda->runInference();
	if (options.debug) std::cout << "Completed LDA inference ..." << std::endl;

	osu::data::DataMat<double> *thetas = lda->getThetas();
	osu::data::DataMat<double> *taus = lda->getTaus();
	std::vector< std::pair<int, double>* > *consensusScores = applyConsensus(thetas, taus);
	//std::sort(consensusScores->begin(), consensusScores->end(), compareSecondDescending<int,double>);
	if (options.debug) std::cout << "Completed consensus ..." << std::endl;

	if (!options.consensus_file.empty()) {
		std::ofstream consensusfile(
				options.consensus_file.c_str() ,
				std::ios::out);
		for (int i = 0; i < options.prefix_cols; i++)
			consensusfile << "col_" << (i+1) << ",";
		consensusfile << "score" << std::endl;
		output_to_file(consensusScores, firstCols, consensusfile);
	}

	osu::releaseVectorElements< std::pair<int, double> >(consensusScores);
	osu::releaseVectorElements< osu::data::SparseVec<int> >(instances);
	delete consensusScores;
	delete lda;
}

void RankAggregator::loadData() {

	osu::data::DataMat<double> *data =
		osu::data::loadCSV<double>(options.data_file,
				options.skip_top_rows,options.skip_left_cols,options.skip_right_cols);

	if (options.prefix_cols > 0) {
		firstCols =
			osu::data::loadCSVCol(options.data_file,
					options.skip_top_rows,0,options.prefix_cols);
	}

	// convert column to rows
	osu::data::DataMat<double> *t_data = transpose<double>(data);
	if (options.debug)
		std::cout << "Loaded data mat...(" << t_data->rows <<
		"," << t_data->cols << ")" << std::endl;

	// sort and rank
	mat = sortAndRank(t_data);
	if (options.debug) std::cout << "Sorted and Ranked data ..." << std::endl;

	normalizeRanks(mat, options);
	if (options.debug) std::cout << "Normalized Ranked data ..." << std::endl;

	if (!options.ranked_file.empty()) {
		std::ofstream rankedfile(
				options.ranked_file.c_str() ,
				std::ios::out);
		osu::output_data(rankedfile, mat->data, mat->rows, mat->cols);
	}
	if (options.debug) std::cout << "Written Ranked data to file..." << std::endl;

	delete t_data;
	delete data;

}

RankAggregator::~RankAggregator() {
	if (mat) delete mat;
	if (uids) delete uids;
	if (firstCols) delete firstCols;
}

}
