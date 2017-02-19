/*
 * testinference.cpp
 *
 *  Created on: Oct 26, 2013
 *      Author: Moy
 */

#include <iostream>
#include <iomanip>
#include <time.h>
#include <fstream>
#include <vector>
#include <map>
#include "../common/utils.h"
#include "../common/Options.h"
#include "../math/math.h"
#include "../data/datamanagement.h"
#include "../topicmodel/simpleLDA.h"
#include "../topicmodel/multinomialNBLDA.h"
#include "../topicmodel/groupedMultinomialNBLDA.h"
#include "../common/Timer.h"
#include "RankAggregator.h"
#include "RankAggregatorPL.h"

void runRankingLDA(osu::Options& options);
void runSimpleLDA(osu::Options& options);
void runMultinomialLDA(osu::Options& options);
void runGroupedMultinomialLDA(osu::Options& options);
void runRankingPL(osu::Options& options);

osu::data::SparseInputData *prepareData(osu::Options& options);

int main( int argc, char** argv )
{
	//if (true) _main(argc, argv);

	osu::Options options;
	if (!options.parse(argc, argv)) {
		return -1;
	}
	options.printOptions();

	osu::TimerCollection *timers = new osu::TimerCollection();
	osu::Timer *timer = osu::startTimer(timers, "main");

	if (options.randomize) {
		//time_t seed_time;
		//time(&seed_time);
		srand(options.randseed);
	}
	switch (options.rankagg) {
	case 1:
		runRankingPL(options);
		break;
	case 2:
		runRankingLDA(options);
		break;
	default:
		std::cout << "ERROR: Invalid rankagg..." << std::endl;
	case 0:
		switch (options.lda) {
		case 0:
			runSimpleLDA(options);
			break;
		case 1:
			runMultinomialLDA(options);
			break;
		case 2:
			runGroupedMultinomialLDA(options);
			break;
		default:
			std::cout << "ERROR: Invalid lda option..." << std::endl;
		}
		break;
	}

	osu::endTimer(timer);
	if (options.time && timers)
		std::cout << "Times::" << (*timers) << std::endl;

	delete timers;

	return 0;
}

int _main( int argc, char** argv )
{
	int test = 2;
	osu::Options options;
	options.epochs = 10;
	options.parse(argc, argv);
	options.printOptions();
	if (options.topics <= 0) options.topics = 3;
	if (options.groups <= 0) options.groups = 3;
	options.sparse = true;
	options.usegroupids = true;
	options.debug = true;
	options.time = true;
	if (options.data_file.empty()) {
		if (options.sparse) {
			options.data_file = "git/topic_models/sampledata/data_true_multinomial-sparse.csv";
			if (options.vocab_file.empty()) {
				options.vocab_file = "git/topic_models/sampledata/data_true_multinomial-vocab.csv";
			}
		} else {
			options.data_file = "git/topic_models/sampledata/data_true_multinomial.csv";
			options.skip_left_cols = 1;
		}
	}
	if (options.thetas_file.empty()) {
		options.thetas_file = "experiments/lda/synthetic_data_thetas.csv";
	}
	if (options.taus_file.empty()) {
		options.taus_file = "experiments/lda/synthetic_data_taus.csv";
	}
	if (options.pis_file.empty()) {
		options.pis_file = "experiments/lda/synthetic_data_pis.csv";
	}
	if (options.debug) std::cout << "Loading data from file: " << options.data_file << std::endl;

	switch (test) {
	case 0:
		runSimpleLDA(options);
		break;
	case 1:
		runMultinomialLDA(options);
		break;
	case 2:
		runGroupedMultinomialLDA(options);
		break;
	case 3:
		runRankingLDA(options);
		break;
	default:
		std::cout << "No valid executable configured..." << std::endl;
	}
	return 0;
}

void runRankingPL(osu::Options& options) {
	osu::RankAggregatorPL ragg(options);
	ragg.loadData();
	ragg.rank();
}

void runRankingLDA(osu::Options& options) {
	osu::RankAggregator ragg(options);
	ragg.loadData();
	ragg.rank();
}

void runSimpleLDA(osu::Options& options) {
	osu::data::SparseInputData *data = prepareData(options);

	osu::lda::SimpleLDA* lda =
			new osu::lda::SimpleLDA(data->data, data->holdoutData, data->userids, options);
	lda->runInference();
	if (options.debug) std::cout << "Completed inference ..." << std::endl;

	delete lda;
	delete data;
}

void runMultinomialLDA(osu::Options& options) {
	osu::data::SparseInputData *data = prepareData(options);

	osu::lda::NbLDA* lda = new osu::lda::NbLDA(data->data, data->holdoutData, data->userids, options);
	lda->runInference();
	if (options.debug) std::cout << "Completed inference ..." << std::endl;

	delete lda;
	delete data;
}

void runGroupedMultinomialLDA(osu::Options& options) {
	osu::data::SparseInputData *data = prepareData(options);

	osu::lda::NbLDA* lda = new osu::lda::GroupedNbLDA(data->data, data->holdoutData, data->groupids, data->userids, options);
	lda->runInference();
	if (options.debug) std::cout << "Completed inference ..." << std::endl;

	delete lda;
	delete data;
}

osu::data::SparseInputData *prepareData(osu::Options& options) {

	osu::data::SparseInputData *inputData = 0;

	if (!options.sparse) {
		osu::data::DataMat<int> *mat =
				osu::data::loadCSV<int>(options.data_file,
						options.skip_top_rows,options.skip_left_cols,
						options.skip_right_cols);
		if (options.debug)
			std::cout << "Loaded data mat...(" << mat->rows << "," << mat->cols << ")" << std::endl;

		std::vector<osu::data::SparseVec<int>*> *instances = osu::lda::convert_to_sparse(mat, options);
		if (options.debug) std::cout << "Initialized sparse vecs ..." << std::endl;

		osu::data::DataMat<int> *uids = osu::data::loadIntCol(
				options.data_file,options.skip_top_rows,0);
		if (options.debug)
			std::cout << "Loaded uid mat...(" << uids->rows << "," << uids->cols << ")" << std::endl;

		inputData = new osu::data::SparseInputData(instances, mat->cols, 0, uids, 0);

		delete mat;
		if (options.debug) std::cout << "Loaded dense data ..." << std::endl;
	} else {
		inputData = osu::data::loadSparseVectors(options.data_file, options.vocab_file, options.numholdout);
		if (options.debug) std::cout << "Loaded sparse data ..." << std::endl;
	}

	return inputData;

}
