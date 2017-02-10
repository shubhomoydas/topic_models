/*
 * multinomialNBLDA.cpp
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
#include "../common/DataArchiver.h"
#include "../math/math.h"
#include "../data/datamanagement.h"
#include "multinomialNBLDA.h"
#include "../common/Timer.h"

namespace osu {
namespace lda {

void NbLDA::runInference() {

	int TN = instances->size();
	int F = instances->front()->cols;
	int T = options.topics;

	if (options.debug) {
		std::cout << "TN: " << TN << ", F: " << F << ", T: " << T
				<< ", holdout: " << (holdout ? holdout->size() : 0) << std::endl;
	}

	clear_params();

	osu::DataArchiver archiver(options);

	// pre-computed information
	std::map<int,int> *id2idx = get_id2idx(uids, options);
	if (options.debug) std::cout << "Initialized id2idx ..." << std::endl;

	int N = id2idx->size();

	// Precompute number of documents for each user
	int *Ni = get_per_user_doc_counts<int>(uids, id2idx, options);
	if (options.debug) std::cout << "Initialized Ni ..." << std::endl;

	int *Nw = osu::lda::get_document_lengths(instances, options);
	if (options.debug) std::cout << "Initialized Nw ..." << std::endl;

	// Initialize the labels of each instance to random topics
	int *Z = osu::zeros<int>(TN);
	double *P = osu::ones<double>(T);
	osu::normalize<double>(P, T);
	//osu::print_arr(P, T);
	osu::generate_multinoimal_samples(Z, TN, P, T);
	if (options.debug) std::cout << "Initialized Z ..." << std::endl;

	// Precompute number of words in each topic (F x T)
	int **Nk = get_word_counts_per_topic(instances, Z, options);
	//osu::msg(options.debug, "Initialized Nk ...", Nk, F, T);

	// Precompute number of sequences in each topic for each user (T x N)
	int **Nn = get_topic_counts_per_user<int>(uids, Z, id2idx, options);
	//osu::msg(options.debug, "Initialized Nn ...", Nn, T, N);

	double *Beta = osu::ones<double>(F);
	osu::normalize(Beta, F);
	double *Alpha = osu::ones<double>(T);

	double sumBetas = osu::sum(Beta, F);
	//double sumAlphas = osu::sum_array<double>(Alpha, T);
	double *sumNT = osu::zeros<double>(T);
	osu::sum_rows(Nk, F, T, sumNT);

	if (options.debug) std::cout << "Completed Initializations ..." << std::endl;

	//if (true) return;

	const int maxEpochs = options.epochs;
	osu::TimerCollection *timers = new osu::TimerCollection();
	osu::Timer *timer = 0;
	// Debug use...
	for (int epoch = 0; epoch < maxEpochs; epoch++) {
		timer = osu::startTimer(timers, "MultinomialNBLDA::ProcessingEpoch");
		int p = 0;
		for (int i = 0; i < N; i++) {
			// Note: Assume the user data instances are contiguous
			// and in correct order in all aggregate data structures like Ni.
			int ni = Ni[i];
			for (int j = 0; j < ni; j++) {

				//if (options.debug) {
				//	char _iterId[100];
				//	sprintf(_iterId, "Epoch: %d; User: %d; Doc: %d",epoch, i, j);
				//	std::string iterId = std::string(_iterId);
				//	osu::msg(options.debug, iterId, sumNT, T);
				//}

				int idx = p+j;

				osu::data::SparseVec<int> *vec = instances->at(idx);

				int z = Z[idx];

				osu::col_add_vals_idxs(Nk, z, vec->vals, vec->idxs, vec->len, -1); // subtract
				sumNT[z] = sumNT[z] - Nw[idx];
				//osu::sum_rows<double>(Nk, F, T, sumNT); // Inefficient to do here...

				Nn[z][i] = Nn[z][i] - 1;

				osu::setVal<double>(P, T, 0);

				//osu::msg(options.debug, "sumNT: ", sumNT, T);

				for (int k = 0; k < T; k++) {
					double sumDenom = sumNT[k] + sumBetas;
					double prod = 1.0;
					double l = 0.0;
					for (int m = 0; m < vec->len; m++) {
						int w  = vec->idxs[m];
						int nv = vec->vals[m];
						for (int v = 0; v < nv; v++) {
							prod = prod * (Nk[w][k] + Beta[w] + v) / (sumDenom + l);
							l = l + 1;
						}
					}
					P[k] = prod * (Nn[k][i]+Alpha[k]);
				}
				//osu::msg(options.debug, "Unnormalized P: ", P, T);
				osu::normalize<double>(P, T);
				//osu::msg(options.debug, "Normalized P: ", P, T);

				int nz = osu::generate_multinoimal(P, T);
				Z[idx] = nz;
				osu::col_add_vals_idxs(Nk, nz, vec->vals, vec->idxs, vec->len, 1); // add
				sumNT[nz] = sumNT[nz] + Nw[idx];
				Nn[nz][i] = Nn[nz][i] + 1;
			}
			p += ni;
		}
		osu::endTimer(timer);
		if (options.debug) {
			if ((epoch+1)%50 == 0) {
				std::cout << "Completed Epoch: " << epoch << std::endl;
				if (options.time && timers)
					std::cout << "Times::" << (*timers) << std::endl;
			}
			//osu::sum_rows<double>(Nk, F, T, sumNT);
			//osu::print_arr<double>(sumNT, T);
			if (epoch >= options.logfromepoch) {
				if (archiver.outputThetas()) {
					theta = get_theta(Nn, Alpha, T, N, options, theta);
					archiver.add(theta->data, theta->rows, theta->cols, 0, epoch);
				}
				if (archiver.outputTaus()) {
					tau = get_tau(&Nk, Beta, T, F, options, tau);
					archiver.add(tau->data, tau->rows, tau->cols, 1, epoch);
				}
			}
		}
	}

	if (archiver.outputThetas()) {
		theta = get_theta(Nn, Alpha, T, N, options, theta);
		archiver.add(theta->data, theta->rows, theta->cols, 0, maxEpochs);
	}
	if (archiver.outputTaus()) {
		tau = get_tau(&Nk, Beta, T, F, options, tau);
		archiver.add(tau->data, tau->rows, tau->cols, 1, maxEpochs);
	}

	if (options.debug) {
		std::cout << "Archived parameters..." << std::endl;
	}
	if (options.time && timers)
		std::cout << "Times::" << (*timers) << std::endl;

	// cleanup
	delete[] Alpha;
	delete[] Beta;
	osu::release_2D_array<int>(Nn, T);
	osu::release_2D_array<int>(Nk, F);
	delete[] Z;
	delete[] P;
	delete[] Nw;
	delete[] sumNT;
	delete[] Ni;
	delete id2idx;
	delete timers;
}

osu::data::DataMat<double> *NbLDA::getThetas() {
	return theta;
}

osu::data::DataMat<double> *NbLDA::getTaus() {
	return tau;
}

osu::data::DataMat<double> *NbLDA::get_tau(int ***Nk,
		double *Beta, int T, int F, osu::Options& options,
		osu::data::DataMat<double> *dest) {
	osu::data::DataMat<double> *Tau = dest;
	if (!Tau) {
		Tau = new osu::data::DataMat<double>(F, T);
	} else {
		osu::setVal<double>(Tau->data, F, T, 0);
	}
	double *sumRows = new double[T];
	osu::sum_rows(Nk[0], F, T, sumRows);
	double sumBetas = osu::sum(Beta, F);
	for (int t = 0; t < T; t++) {
		osu::col_add(Tau->data, t, Nk[0], t, F);
		osu::col_add(Tau->data, t, Beta, F);
		osu::col_multiply(Tau->data, t, F, 1.0/(sumBetas+sumRows[t]));
	}
	delete[] sumRows;
	return Tau;
}
osu::data::DataMat<double> *NbLDA::get_theta(int **Nn,
		double *Alpha, int T, int N, osu::Options& options,
		osu::data::DataMat<double> *dest) {
	osu::data::DataMat<double> *Theta = dest;
	if (!Theta) {
		Theta = new osu::data::DataMat<double>(T, N);
	} else {
		osu::setVal<double>(Theta->data, T, N, 0);
	}
	double *sumRows = new double[N];
	osu::sum_rows(Nn, T, N, sumRows);
	double sumAlphas = osu::sum(Alpha, T);
	for (int i = 0; i < N; i++) {
		osu::col_add(Theta->data, i, Nn, i, T);
		osu::col_add(Theta->data, i, Alpha, T);
		osu::col_multiply(Theta->data, i, T, 1.0/(sumAlphas+sumRows[i]));
	}
	delete[] sumRows;
	return Theta;
}

void NbLDA::clear_params() {
	if (theta) {
		delete theta;
		theta = 0;
	}
	if (tau) {
		delete tau;
		tau = 0;
	}
}

std::map<int,int> *NbLDA::get_id2idx(
		osu::data::DataMat<int> *uids, osu::Options& options) {
	std::map<int,int> *id2idx = new std::map<int,int>();
	int maxidx = 0;
	for (int i = 0; i < uids->cols; i++) {
		int id = uids->data[0][i];
		std::map<int,int>::iterator itr = id2idx->find(id);
		if (itr == id2idx->end()) {
			id2idx->insert(std::pair<int,int>(id, maxidx));
			maxidx++;
		}
	}
//	for (std::map<int,int>::iterator itr = id2idx->begin();
//			itr != id2idx->end(); itr++) {
//		std::cout << itr->first << " -> " << itr->second << std::endl;
//	}
	return id2idx;
}

std::map<int,int> *NbLDA::get_idx2id(
		std::map<int,int> *id2idx, osu::Options& options) {
	std::map<int,int> *idx2id = new std::map<int,int>();
	for (std::map<int,int>::iterator it = id2idx->begin(); it != id2idx->end(); it++) {
		idx2id->insert(std::pair<int,int>(it->second, it->first));
	}
	return idx2id;
}

}
}
