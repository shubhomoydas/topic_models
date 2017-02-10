/*
 * multinomialNBLDA.cpp
 *
 *  Created on: Oct 26, 2013
 *      Author: Moy
 */

#include <float.h>
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
#include "simpleLDA.h"
#include "../common/Timer.h"

namespace osu {
namespace lda {

void SimpleLDA::runInference() {

	int N = instances->size(); // Total number of documents
	int F = instances->front()->cols;  // Total number of words in vocabulary
	int T = options.topics; // Number of topics

	if (options.debug) {
		std::cout << "N: " << N << ", F: " << F << ", T: " << T
				<< ", holdout: " << (holdout ? holdout->size() : 0) << std::endl;
	}

	clear_params();

	osu::DataArchiver archiver(options);

	// pre-computed information

	int *Nw = osu::lda::get_document_lengths(instances, options);
	if (options.debug) std::cout << "Initialized Nw ..." << std::endl;
	int W = osu::sum(Nw, N); // Total word count across all documents.
	if (options.debug) std::cout << "W: " << W << std::endl;

	// Initialize the labels of each instance to random topics
	int *Z = osu::zeros<int>(W);
	double *P = osu::ones<double>(T);
	osu::normalize<double>(P, T);
	osu::generate_multinoimal_samples(Z, W, P, T);
	if (options.debug) std::cout << "Initialized Z ..." << std::endl;

	// Precompute number of words in each topic (F x T)
	int **Nk = get_word_counts_per_topic(instances, Z, options);
	//osu::msg(options.debug, "Initialized Nk ...", Nk, F, T);

	// Precompute topic counts in each document (T x N)
	int **Nm = get_topic_counts_per_document(instances, Z, options);
	//osu::msg(options.debug, "Initialized Nm ...", Nm, T, N);

	double *Beta = osu::ones<double>(F);
	osu::normalize(Beta, F);
	double *Alpha = osu::ones<double>(T);
	osu::normalize(Beta, F);
	osu::normalize(Alpha, T);

	double sumBetas = osu::sum(Beta, F);
	//double sumAlphas = osu::sum_array<double>(Alpha, T);
	int *sumNT = osu::zeros<int>(T);
	osu::sum_rows(Nk, F, T, sumNT);

	if (theta) delete theta;
	if (tau) delete tau;
	theta = 0;
	tau = 0;

	if (options.debug) std::cout << "Completed Initializations ..." << std::endl;

	//if (true) return;

	const int maxEpochs = options.epochs;
	osu::TimerCollection *timers = new osu::TimerCollection();
	osu::Timer *timer = 0;
	// Debug use...
	//for (int epoch = 0; epoch < 1; epoch++) {
	for (int epoch = 0; epoch < maxEpochs; epoch++) {
		timer = osu::startTimer(timers, "SimpleLDA::ProcessingEpoch");
		int *zptr = Z;
		for (int i = 0; i < N; i++) {
			osu::data::SparseVec<int> *vec = instances->at(i);
			for (int iv = 0; iv < vec->len; iv++) {
				int v = vec->idxs[iv];
				int nw = vec->vals[iv];
				for (int w = 0; w < nw; w++) {
					int z = *zptr;

					Nk[v][z] = Nk[v][z] - 1;
					Nm[z][i] = Nm[z][i] - 1;
					sumNT[z] = sumNT[z] - 1;

					for (int k = 0; k < T; k++) {
						P[k] = (Nk[v][k] + Beta[v])*(Nm[k][i] + Alpha[k])
								/(sumNT[k]+sumBetas);
					}
					osu::normalize(P, T);

					int nz = osu::generate_multinoimal(P, T);

					//std::cout << "i: " << i << "; v: " << v <<
					//		"; z: " << z << "; nz: " << nz <<
					//		"; Nk[v][z]: " << Nk[v][z] << "; Nm[z][i]: " << Nm[z][i] << std::endl;

					Nk[v][nz] = Nk[v][nz] + 1;
					Nm[nz][i] = Nm[nz][i] + 1;
					sumNT[nz] = sumNT[nz] + 1;
					*zptr = nz;

					zptr++;
				}
			}
		}
		osu::endTimer(timer);
		if (options.debug) {
			if ((epoch+1)%50 == 0) {
				std::cout << "Completed Epoch: " << epoch << std::endl;
				if (options.time && timers)
					std::cout << "Times::" << (*timers) << std::endl;
			}
			if (epoch >= options.logfromepoch) {
				//std::cout << "Computing Parameters in epoch " << epoch << std::endl;
				if (archiver.outputThetas()) {
					theta = get_theta(Nm, Alpha, T, N, options, theta);
					archiver.add(theta->data, theta->rows, theta->cols, 0, epoch);
				}
				if (archiver.outputTaus()) {
					tau = get_tau(&Nk, Beta, T, F, options, tau);
					archiver.add(tau->data, tau->rows, tau->cols, 1, epoch);
				}
			}
			//osu::sum_rows<double>(Nk, F, T, sumNT);
			//osu::print_arr<double>(sumNT, T);
		}
	}

	if (archiver.outputThetas()) {
		theta = get_theta(Nm, Alpha, T, N, options, theta);
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
	osu::release_2D_array<int>(Nk, F);
	osu::release_2D_array<int>(Nm, T);
	delete[] Z;
	delete[] P;
	delete[] Nw;
	delete[] sumNT;
	delete timers;
}

osu::data::DataMat<double> *SimpleLDA::getThetas() {
	return theta;
}

osu::data::DataMat<double> *SimpleLDA::getTaus() {
	return tau;
}

/**
 * Perplexity is:
 *   exp(-(sum_{d=1..M}(log(p(w_d)))) / (sum_{d=1..M}(N_d))
 * where:
 *   M - number of documents in test set
 *   w_d - all words in document d
 *     p(w_d) - prod_{n=1..N_d}(sum_{z=1..K}(p(w_n|z)p(z|d)p(d)))
 *   N_d - Total number of words in document d
 */
double SimpleLDA::getPerplexity(std::vector<osu::data::SparseVec<int>*> *testInstances) {
	double perplexity = 0;
	if (!theta || !tau) return 0;
	// TODO Later
	return perplexity;
}

void SimpleLDA::clear_params() {
	if (theta) {
		delete theta;
		theta = 0;
	}
	if (tau) {
		delete tau;
		tau = 0;
	}
}

osu::data::DataMat<double> *SimpleLDA::get_tau(int ***Nk,
		double *Beta, int T, int F, osu::Options& options,
		osu::data::DataMat<double> *dest) {
	osu::data::DataMat<double> *Tau = dest;
	if (!Tau) {
		Tau = new osu::data::DataMat<double>(F, T);
	}
	osu::setVal<double>(Tau->data, F, T, 0);
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
osu::data::DataMat<double> *SimpleLDA::get_theta(int **Nm,
		double *Alpha, int T, int N, osu::Options& options,
		osu::data::DataMat<double> *dest) {
	osu::data::DataMat<double> *Theta = dest;
	if (!Theta) {
		Theta = new osu::data::DataMat<double>(T, N);
	}
	osu::setVal<double>(Theta->data, T, N, 0);
	double *sumRows = new double[N];
	osu::sum_rows(Nm, T, N, sumRows);
	double sumAlphas = osu::sum(Alpha, T);
	for (int i = 0; i < N; i++) {
		osu::col_add(Theta->data, i, Nm, i, T);
		osu::col_add(Theta->data, i, Alpha, T);
		osu::col_multiply(Theta->data, i, T, 1.0/(sumAlphas+sumRows[i]));
	}
	delete[] sumRows;
	return Theta;
}

int **SimpleLDA::get_topic_counts_per_document(
		osu::data::DataMat<int> *mat, int *Z, osu::Options& options) {

	int F = mat->cols;
	int T = options.topics;
	int N = mat->rows;
	int **Nm = osu::zeros<int>(T, N);
	int *zptr = Z;
	for (int i = 0; i < N; i++) {
		int *fv = mat->data[i];
		for (int v = 0; v < F; v++) {
			int nw = fv[v];
			for (int w = 0; w < nw; w++) {
				Nm[*zptr][i] = Nm[*zptr][i] + 1;
				zptr++;
			}
		}
	}

	return Nm;

}

int **SimpleLDA::get_word_counts_per_topic(
		osu::data::DataMat<int> *mat, int *Z, osu::Options& options) {
	int F = mat->cols;
	int T = options.topics;
	int N = mat->rows;
	int **Nk = osu::zeros<int>(F,T);
	int *zptr = Z;
	for (int i = 0; i < N; i++) {
		int *fv = mat->data[i];
		for (int v = 0; v < F; v++) {
			int nw = fv[v];
			for (int w = 0; w < nw; w++) {
				Nk[v][*zptr] = Nk[v][*zptr] + 1;
				zptr++;
			}
		}
	}
	return Nk;
}

}
}
