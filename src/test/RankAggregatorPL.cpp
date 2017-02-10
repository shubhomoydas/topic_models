/*
 * RankAggregatorPL.cpp
 *
 *  Created on: Jun 21, 2014
 *      Author: Moy
 */

#include<fstream>
#include <utility>
#include <vector>
#include <float.h>
#include <cmath>
#include "../data/datamanagement.h"
#include "../common/Timer.h"
#include "../common/DataArchiver.h"
#include "../math/math.h"
#include "RankAggregatorPL.h"

namespace osu {

/**
 * We assume that the input data are scores (NOT ranks) from
 * anomaly detectors. Each column pertains to one detector and
 * the rows are ordered by instance ids. The first thing we do is
 * sort and rank the data.
 */
void RankAggregatorPL::loadData() {

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
	mat = sortAndOrder(t_data);
	if (options.debug) std::cout << "Sorted and Ranked data ..." << std::endl;

	if (!options.ranked_file.empty()) {
		std::ofstream rankedfile(
				options.ranked_file.c_str() ,
				std::ios::out);
		osu::output_data(rankedfile, mat->data, mat->rows, mat->cols);
	}
	if (options.debug) std::cout << "Written Ranked data to file..." << std::endl;

	// adjust the item ids to start from 0
	for (int i = 0; i < mat->rows; i++) {
		osu::add(mat->data[i], mat->cols, -1);
	}

	delete t_data;
	delete data;

}

void RankAggregatorPL::loadRankedData() {

	mat =
		osu::data::loadCSV<int>(options.data_file,
				options.skip_top_rows,options.skip_left_cols,options.skip_right_cols);

	// adjust the item ids to start from 0
	for (int i = 0; i < mat->rows; i++) {
		osu::add(mat->data[i], mat->cols, -1);
	}

	//std::cout << "Data: " << std::endl;
	//osu::print_arr(mat->data, mat->rows, mat->cols);

	if (options.debug) std::cout << "Data Loaded: matrix (" <<
			mat->rows << "," << mat->cols << ")..." << std::endl;

	if (options.prefix_cols > 0) {
		std::cout << "Option prefix_cols not used for PL ranking" << std::endl;
	}

}

int **getOrderedRanks(osu::data::DataMat<int> *mat, int D, int N) {
	int **Ordered_ranks = osu::zeros<int>(D,N);
	int **data = mat->data;
	for (int d = 0; d < D; d++) {
		int *data_d = data[d];
		for (int i = 0; i < N; i++) {
			Ordered_ranks[d][data_d[i]] = i;
		}
	}
	return Ordered_ranks;
}

void RankAggregatorPL::rank() {

	int **S = mat->data;
	int D = mat->rows;
	int N = mat->cols;
	int K = options.groups;

	if (options.debug) std::cout << "D=" << D << ", N=" << N << ", K=" << K << std::endl;

	osu::TimerCollection *timers = new osu::TimerCollection();
	osu::Timer *timer = 0;
	osu::DataArchiver archiver(options);

	double **Z = osu::ones<double>(D, K);
	for (int d = 0; d < D; d++) {
		osu::normalize(Z[d], K);
	}

	double *Pi = osu::zeros<double>(K);
	osu::sum_rows(Z, D, K, Pi);
	osu::normalize(Pi, K);

	double *GsPrior = osu::ones<double>(N);
	osu::normalize(GsPrior, N);
	double **Gs = osu::zeros<double>(K,N);
	for (int k = 0; k < K; k++) {
		osu::generate_uniform_prob_vector(Gs[k], N);
	}
	delete[] GsPrior;

	int **Ordered_ranks = getOrderedRanks(mat, D, N);
	//std::cout << "Ordered Ranks..." << std::endl;
	//osu::print_arr(Ordered_ranks, D, N);

	if (options.debug) std::cout << "After initialization ..." << std::endl;

	double **Gs_sum = osu::zeros<double>(D,N);
	double **Gs_tmp = osu::zeros<double>(K,N);
	double ll_prev = -FLT_MAX;
	double ll_diff = ll_prev;
	int maxepochs = options.epochs;
	//LL = [];
	for (int epoch = 0; epoch < maxepochs; epoch++) {

		timer = osu::startTimer(timers, "RankAggregatorPL::rank");
	    // M-Step
	    // ------

	    // MLE of Pi
		osu::sum_rows(Z, D, K, Pi);
		osu::normalize(Pi, K);

	    // MLE of v_kn
	    for (int k = 0; k < K; k++) {
	        for (int d = 0; d < D; d++) {
	        	computeCumsum(S, Gs, Gs_sum, k, d, N);
	        }
	        for (int n = 0; n < N; n++) {
	            // we assume that we get complete rankings on N items from
	            // each detector. Therefore, we can simplify the indicator
	            // function in the numerator of the derivation.
	            double nv = osu::col_sum(Z,D,k);
	            double tv = 0;
	    		//if (options.debug) std::cout << "M-Step 3>..." << std::endl;
	            for (int d = 0; d < D; d++) {
	                double s_l = 0;
	                for (int i = 0; i < N; i++) {
	                	// this check is equivalent to the 'which'
	                    if (Ordered_ranks[d][n] >= i) {
	                        s_l = s_l + (1/Gs_sum[d][i]);
	                    }
	                }
	                tv = tv + Z[d][k]*s_l;
	            }
	            Gs_tmp[k][n] = nv / tv;
	    		//if (options.debug) std::cout << "M-Step 3<..." << std::endl;
	        }
	    }
	    osu::copy(Gs_tmp, Gs, K, N);

	    // E-Step
	    // ------
	    if (K > 1) {
	        // TODO: We need to use logarithms for product and normalize for large
	        // N
            for (int k = 0; k < K; k++) {
				for (int d = 0; d < D; d++) {
		        	computeCumsum(S, Gs, Gs_sum, k, d, N);
	                double tz = std::log(Pi[k]);
	                for (int i = 0; i < N; i++) {
	                    tz = tz + std::log(Gs[k][S[d][i]] / Gs_sum[d][i]);
	                }
	                Z[d][k] = tz;
	            }
	        }
	        for (int d = 0; d < D; d++) {
	            double mean = osu::sum(Z[d], K) / K;
	            osu::add(Z[d], K, -mean);
	            double mx = osu::max(Z[d], K);
	            if (mx > 700) {
	            	osu::add(Z[d], K, -(mx-700.0));
	            }
	            osu::exp(Z[d], K);
	            osu::normalize(Z[d], K);
	        }
	    }

	    // compute log-likelihood for stopping criteria
	    double ll = computePLLogLik(S, Gs, Pi, Z, K, D, N);
	    //LL = [LL; ll];
	    ll_diff = ll-ll_prev;

	    if (options.debug && epoch % 1 == 0) {
	        //Gs_saved = [Gs_saved;[ones(N,1)*epoch,Gs]];
	        std::cout << "epoch = " << epoch << ", ll_prev = " << ll_prev <<
	        		", ll = " << ll << ", ll diff = " << ll_diff << std::endl;
	    }

	    ll_prev = ll;
	    if (abs(ll_diff) < 1e-2) {
	        std::cout << "Exiting at epoch = " << epoch << ", ll_prev = " << ll_prev <<
	        		", ll = " << ll << ", ll diff = " << ll_diff << std::endl;
	        break;
	    }

		osu::endTimer(timer);
		if (options.debug && options.time && timer) {
			std::cout << "Timer: " << (*timer) << std::endl;
		}
	}

	if (options.time && timers)
		std::cout << "Times::" << (*timers) << std::endl;

	if (archiver.outputGammas()) {
		double **tGs = transpose(Gs, K, N);
		archiver.add(tGs, N, K, 4, maxepochs);
		osu::release_2D_array<double>(tGs, N);
		if (options.debug) std::cout << "Written Gammas to file ..." << std::endl;
	}

	std::vector< std::pair<int, double>* > *consensusScores =
			new std::vector< std::pair<int, double>* >();

	for (int i = 0; i < N; i++) {
		double maxprob = 0;
		for (int k = 0; k < K; k++) {
			if (Gs[k][i] > maxprob)
				maxprob = Gs[k][i];
		}
		consensusScores->push_back(new std::pair<int, double>(i+1,maxprob));
	}

	if (options.debug) std::cout << "Completed PL ranking ..." << std::endl;

	if (!options.consensus_file.empty()) {
		std::ofstream consensusfile(
				options.consensus_file.c_str() ,
				std::ios::out);
		for (int i = 0; i < options.prefix_cols; i++)
			consensusfile << "col_" << (i+1) << ",";
		consensusfile << "score" << std::endl;
		output_to_file(consensusScores, firstCols, consensusfile);
		if (options.debug) std::cout << "Written PL ranking to output file ..." << std::endl;
	}

	osu::releaseVectorElements< std::pair<int, double> >(consensusScores);
	delete[] Pi;
	osu::release_2D_array<double>(Z, D);
	osu::release_2D_array<double>(Gs, K);
	osu::release_2D_array<double>(Gs_sum, D);
	osu::release_2D_array<double>(Gs_tmp, K);
	osu::release_2D_array<int>(Ordered_ranks, D);
	delete consensusScores;
	delete timers;
}

void RankAggregatorPL::computeCumsum(int **S, double **Gs, double **Gs_sum, int k, int d, int N) {
    double cumsum = 0.0;
    for (int i = N-1; i >= 0; i--) {
    	cumsum += Gs[k][S[d][i]];
    	Gs_sum[d][i] = cumsum;
    }
}

double RankAggregatorPL::computePLLogLik(
		int **S, double **Gs, double *Pi,
		double **Z, int K, int D, int N ) {
    double ll = 0;
    double **Gs_sum = osu::zeros<double>(D,N);
    for (int k = 0; k < K; k++) {
        for (int d = 0; d < D; d++) {
        	computeCumsum(S, Gs, Gs_sum, k, d, N);
        }
        for (int d = 0; d < D; d++) {
            ll = ll + Z[d][k] * std::log(Pi[k]);
            for (int i = 0; i < N; i++) {
            	ll = ll + Z[d][k] * std::log(Gs[k][S[d][i]] / Gs_sum[d][i]);
            }
        }
    }
	osu::release_2D_array<double>(Gs_sum, D);
    return ll;
}

void RankAggregatorPL::output_to_file(std::vector< std::pair<int, double>* > *data,
		std::vector<std::string> *pre, std::ofstream& out) {
	std::vector< std::pair<int, double>* >::iterator it = data->begin();
	for (int i = 0; it != data->end(); i++, it++) {
		if (pre) out << pre->at(i) << ",";
		//out << (*it)->first << "," << (*it)->second << std::endl;
		out << (*it)->second << std::endl;
	}
}

void RankAggregatorPL::rankSimple() {
	std::cout << "ERROR: method RankAggregatorPL::rankSimple() not implemented..." << std::endl;
}

void RankAggregatorPL::rankLDA() {
	std::cout << "ERROR: method RankAggregatorPL::rankLDA() not implemented..." << std::endl;
}

RankAggregatorPL::~RankAggregatorPL() {
	if (mat) delete mat;
	if (uids) delete uids;
	if (firstCols) delete firstCols;
}

}
