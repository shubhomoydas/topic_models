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
#include "groupedMultinomialNBLDA.h"
#include "../common/Timer.h"

const double PROD_THRESH = 1.0e-140; // some very small number...

namespace osu {
namespace lda {

void GroupedNbLDA::runInference() {

	int TN = instances->size();
	int F = instances->front()->cols;
	int T = options.topics;
	int K = options.groups;

	osu::TimerCollection *initTimers = 0;
	osu::Timer *initTimer = 0;
	if (options.time) initTimers = new osu::TimerCollection();

	if (options.debug) {
		std::cout << "TN: " << TN << ", F: " << F << ", T: " << T
				<< ", K: " << K << ", holdout: " << (holdout ? holdout->size() : 0) << std::endl;
	}

	clear_params();

	osu::DataArchiver archiver(options);

	// pre-computed information
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_id2idx");
	std::map<int,int> *id2idx = get_id2idx(uids, options);
	std::map<int,int> *idx2id = get_idx2id(id2idx, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized id2idx ..." << std::endl;

	std::map<int,int> *groupid2idx = 0;
	if (useFixedGroupIds && groupids->size() != id2idx->size()) {
		// The number of users is different from the
		// number of users for whom the groups have been provided.
		useFixedGroupIds = false;
	} else if (useFixedGroupIds) {
		groupid2idx = get_groupid2idx(groupids, options);
	}

	int N = id2idx->size();
	if (options.debug) std::cout << "N = " << N << std::endl;

	// Initialize the cluster membership for each user to random
	int *Y = osu::zeros<int>(N);
	double *PY = osu::ones<double>(K);
	double *PYlog = osu::zeros<double>(K);
	osu::normalize<double>(PY, K);
	//osu::print_arr(PY, K);

	if (useFixedGroupIds) {
		populateFixedGroups(Y, groupids, id2idx, groupid2idx);
		if (options.debug) std::cout << "Initialized Y from fixed ..." << std::endl;
	} else {
		osu::generate_multinoimal_samples(Y, N, PY, K);
		if (options.debug) std::cout << "Initialized Y as random ..." << std::endl;
	}

	// Initialize the labels of each instance to random topics
	int *Z = osu::zeros<int>(TN);
	double *PZ = osu::ones<double>(T);
	osu::normalize<double>(PZ, T);
	//osu::print_arr(P, T);
	osu::generate_multinoimal_samples(Z, TN, PZ, T);
	if (options.debug) std::cout << "Initialized Z ..." << std::endl;

	// Precompute number of documents for each user
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_per_user_doc_counts");
	int *Ni = get_per_user_doc_counts<int>(uids, id2idx, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Ni ..." << std::endl;

	// Precompute number of words in each topic (F x T)
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_user_counts_per_group");
	int *Uk = get_user_counts_per_group<int>(Y, N, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Uk ..." << std::endl;
	//osu::msg(options.debug, "Initialized Uk ...", Uk, K);

	// Precompute number of documents in each topic for each user (T x N)
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_topic_counts_per_user");
	int **Mti = get_topic_counts_per_user<int>(uids, Z, id2idx, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Mti ..." << std::endl;
	//osu::msg(options.debug, "Initialized Mti ...", Mti, T, N);

	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_document_lengths");
	int *Nw = osu::lda::get_document_lengths(instances, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Nw ..." << std::endl;

	// Precompute number of words in each topic (F x T)
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_word_counts_per_topic");
	int **Nk = get_word_counts_per_topic(instances, Z, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Nk ..." << std::endl;
	//osu::msg(options.debug, "Initialized Nk ...", Nk, F, T);

	// Precompute the number of documents per group per topic (K,T)
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_documents_per_group_topic");
	int **Mkt = get_documents_per_group_topic<int>(Y, Z, id2idx, Ni, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Mkt ..." << std::endl;
	//osu::msg<double>(options.debug, "Initialized Mkt ...", Mkt, K, T);

	// Precompute the number of words per group per topic (K,T)
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_words_per_group_topic");
	int **Nkt = get_words_per_group_topic<int>(Y, Z, id2idx, Ni, Nw, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Nkt ..." << std::endl;
	//osu::msg<double>(options.debug, "Initialized Nkt ...", Nkt, K, T);

	// Precompute the number of words per user per topic (N,T)
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_words_per_user_topic");
	int **Nit = get_words_per_user_topic<int>(Y, Z, id2idx, Ni, Nw, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Nit ..." << std::endl;
	//osu::msg<double>(options.debug, "Initialized Nit ...", Nit, N, T);

	// Precompute the count of each word per topic per group (K,F,T)
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_counts_per_group_word_topic");
	int ***Nkvt = get_counts_per_group_word_topic(instances, Y, Z, id2idx, Ni, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Nkvt ..." << std::endl;
	//osu::msg<double>(options.debug, "Initialized Nkvt ...", Nkvt, K, F, T);

	// Precompute the count of each word per topic per user (N,F,T)
	//int ***Nivt = get_counts_per_user_word_topic(instances, Y, Z, id2idx, Ni, options);
	initTimer = osu::startTimer(initTimers, "GroupedNbLDA::get_sparse_counts_per_user_word_topic");
	osu::data::SparsePagedData<int> *Nivt = get_sparse_counts_per_user_word_topic(instances, Y, Z, id2idx, Ni, options);
	initTimer->end();
	if (options.debug) std::cout << "Initialized Nivt ..." << std::endl;
	//osu::msg<double>(options.debug, "Initialized Nivt ...", Nivt, N, F, T);

	double *Beta = osu::ones<double>(F);
	osu::normalize(Beta, F);
	double *Alpha = osu::ones<double>(T);
	double *Eta = osu::ones<double>(K);

	double sumBetas = osu::sum(Beta, F);
	double sumAlphas = osu::sum(Alpha, T);

	if (options.debug) std::cout << "Completed Initializations ..." << std::endl;
	if (options.time && initTimers)
		std::cout << "Initialization Times::" << (*initTimers) << std::endl;

	if (options.debug) {
		std::cout << "Initial Counts: "<< std::endl;
		if (false) {
			std::cout << "Mtk: " << osu::sum(Mkt, K, T)
					<< "; Nkt: " << osu::sum(Nkt, K, T)
					<< "; Mti: " << osu::sum(Mti, T, N)
					<< "; Nit: " << osu::sum(Nit, N, T)
					<< "; Nkvt: " << osu::sum(Nkvt, K, F, T)
					//<< "; Nivt: " << osu::sum(Nivt, N, F, T)
			<< std::endl;
		}
	}
	//if (true) return;

	osu::TimerCollection *timers = new osu::TimerCollection();
	osu::Timer *timer = 0;

	const int maxEpochs = options.epochs;
	// Debug use...
	for (int epoch = 0; epoch < maxEpochs; epoch++) {
		timer = osu::startTimer(timers, "GroupedNbLDA::ProcessingEpoch");
		int p = 0;
		for (int i = 0; i < N; i++) {
			// Note: Assume the user data instances are contiguous
			// and in correct order in all aggregate data structures like Ni.
			int ni = Ni[i];
			for (int j = 0; j < ni; j++) {

				//if (options.debug) {
				//	char _iterId[100];
				//	sprintf(_iterId, "Epoch: %d; User: %d; Doc: %d",epoch, i, j);
				//}

	            int k = Y[i]; // group of current user

	            if (!useFixedGroupIds) {

					//timer = osu::startTimer(timers, "GroupedLDA::GibbsForGroup");
					/**-------------------------
					 * Gibbs Sampling for group
					 * -------------------------
					 */

					//MATLAB: Mkt[k,:] = Mkt[k,:] - Mti[:,i];
					osu::add_row_col<int>(Mkt, k, Mti, i, T, -1);
					Uk[k] = Uk[k] - 1;

					// Compute p(yi = k | Y_(-i), Zi, Z_(-i), W)
					for (int kk = 0; kk < K; kk++) {
						PY[kk] = 0;
						//0:(sum(Mti(:,i))-1)
						int *Mkt_kk = Mkt[kk];
						double mti = 0;
						//sum( Mkt(kk,:) + Alpha' );
						double pd = osu::sum(Mkt_kk, T) + sumAlphas;
						double prodAcc = (Uk[kk]+Eta[kk]);
						for (int t = 0; t < T; t++) {
							int docsintopic_i = Mti[t][i];
							for (int it = 0; it < docsintopic_i; it++) {
								//--pn(1,tpos:(tpos+Mti(t,i)-1)) = ...
								//--    ( 0:(Mti(t,i)-1) ) + ( Mkt(kk,t) + Alpha(t) );
								//PY[kk] = PY[kk] + log((it+Mkt[kk][t]+Alpha[t])/(pd+mti));
								prodAcc = prodAcc * (it+Mkt_kk[t]+Alpha[t])/(pd+mti);
								if (prodAcc < 0) {
									std::map<int,int>::iterator idit = idx2id->find(i);
									std::cout << "Error:: prodAcc=" << prodAcc << ", useridx: " << i
											<< ", userid: " << (idit == idx2id->end() ? -1 : idit->second)
											<< ", epoch: " << epoch
											<< "Mkt_kk[t]: " << Mkt_kk[t]
											<< ", (pd+mti): " << (pd+mti)
											<< "(it+Mkt_kk[t]+Alpha[t]): " << (it+Mkt_kk[t]+Alpha[t]) << std::endl;
								}
								if (prodAcc < PROD_THRESH) {
									//PY[kk] = PY[kk] + log(prodAcc);
									// Use logarithms for numerical stability...
									PY[kk] += log(prodAcc);
									prodAcc = 1.0;
								}
								mti = mti + 1;
							}
						}
						PY[kk] += log(prodAcc);
					}
					double mean = osu::sum(PY, K) / K;
					osu::add(PY, K, -mean);
					double maxPY = osu::max(PY, K);
					if (maxPY > 700.0)
						osu::add(PY, K, -(maxPY-700.0));
					osu::exp(PY, K);
					osu::normalize(PY, K);

					// Sample new group k
					int nk = osu::generate_multinoimal(PY, K);
					if (nk < 0) {
						std::map<int,int>::iterator idit = idx2id->find(i);
						std::cout << "Error:: nk=" << nk << ", useridx: " << i
								<< ", userid: " << (idit == idx2id->end() ? -1 : idit->second)
								<< ", epoch: " << epoch << ", mean: " << mean << ", maxPY" << maxPY
								<< ", PY: " << std::endl;
						osu::print_arr(PY, K);
						if (options.time && timers)
							std::cout << "Times::" << (*timers) << std::endl;
					}

					// Adjust the values to new k
					// MATLAB: Mkt(nk,:) = Mkt(nk,:) + Mti(:,i)';
					osu::add_row_col<int>(Mkt, nk, Mti, i, T, 1);
					Uk[nk] = Uk[nk] + 1;
					Y[i] = nk;

					// In case a group changes, other pre-computed values need adjustment.
					// subtract from old group
					//Nkt(k,:) = Nkt(k,:) - Nit(i,:);
					osu::add_rows(Nkt, k, Nit, i, T, Nkt[k], -1);
					//Nvtk(:,:,k) = Nvtk(:,:,k) - Nvti(:,:,i);
					//osu::add(Nkvt[k], Nivt[i], F, T, -1);
					Nivt->addToArrayT(i, Nkvt[k], -1);

					// add to new group
					//Nkt(nk,:) = Nkt(nk,:) + Nit(i,:);
					osu::add_rows(Nkt, nk, Nit, i, T, Nkt[nk], 1);
					//Nvtk(:,:,nk) = Nvtk(:,:,nk) + Nvti(:,:,i);
					//osu::add(Nkvt[nk], Nivt[i], F, T, 1);
					Nivt->addToArrayT(i, Nkvt[nk], 1);

					k = nk;

					//if (options.debug) {
					//	std::cout << "Completed Gibbs for Group in Epoch: " <<
					//			epoch << "; User: " << i << std::endl;
					//}

					//osu::endTimer(timer);

	            }

				//timer = osu::startTimer(timers, "GroupedLDA::GibbsForTopic");
				/**-------------------------
				 * Gibbs Sampling for topic
				 * -------------------------
				 */

				int idx = p+j;

				osu::data::SparseVec<int> *vec = instances->at(idx);

				int z = Z[idx];
				osu::data::SparseVec<int> *Nivt_it = Nivt->at(i,z);

				osu::col_add_vals_idxs(Nk, z, vec->vals, vec->idxs, vec->len, -1); // subtract

	            // Adjusted word counts for z-th topic excluding curr doc
				//Nvtk(m,z,k) = Nvtk(m,z,k) - sv;
	            //Nvti(m,z,i) = Nvti(m,z,i) - sv;
				osu::col_add_vals_idxs(Nkvt[k], z, vec->vals, vec->idxs, vec->len, -1);
				//osu::col_add_vals_idxs(Nivt[i], z, vec->vals, vec->idxs, vec->len, -1);
				Nivt_it->add(vec, -1);
	            Nkt[k][z] = Nkt[k][z] - Nw[idx];
	            Nit[i][z] = Nit[i][z] - Nw[idx];

	            Mkt[k][z] = Mkt[k][z] - 1;
				Mti[z][i] = Mti[z][i] - 1;

				osu::setVal<double>(PZ, T, 0);

				for (int t = 0; t < T; t++) {
					double sumDenom = Nkt[k][t] + sumBetas;
					double prod = (Mti[t][i]+Alpha[t]);
					double l = 0.0;
					for (int m = 0; m < vec->len; m++) {
						int w  = vec->idxs[m];
						int nv = vec->vals[m];
						for (int v = 0; v < nv; v++) {
							prod = prod * (Nkvt[k][w][t] + Beta[w] + v) / (sumDenom + l);
							l = l + 1;
						}
					}
					PZ[t] = prod;
				}
				//osu::msg(options.debug, "Unnormalized P: ", P, T);
				osu::normalize<double>(PZ, T);
				//osu::msg(options.debug, "Normalized P: ", P, T);

				int nz = osu::generate_multinoimal(PZ, T);
				if (nz < 0) {
					std::cout << "Error:: nz=" << nz << ", useridx: " << i << ", epoch: " << epoch << std::endl;
					if (options.time && timers)
						std::cout << "Times::" << (*timers) << std::endl;
				}
				Nivt_it = Nivt->at(i,nz);
				Z[idx] = nz;

				osu::col_add_vals_idxs(Nkvt[k], nz, vec->vals, vec->idxs, vec->len, 1);
				//osu::col_add_vals_idxs(Nivt[i], nz, vec->vals, vec->idxs, vec->len, 1);
				Nivt_it->add(vec, 1);
	            Nkt[k][nz] = Nkt[k][nz] + Nw[idx];
	            Nit[i][nz] = Nit[i][nz] + Nw[idx];

	            Mkt[k][nz] = Mkt[k][nz] + 1;
				Mti[nz][i] = Mti[nz][i] + 1;

				//osu::endTimer(timer);
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
			if (false) {
				std::cout << "Mtk: " << osu::sum(Mkt, K, T)
						<< "; Nkt: " << osu::sum(Nkt, K, T)
						<< "; Mti: " << osu::sum(Mti, T, N)
						<< "; Nit: " << osu::sum(Nit, N, T)
						<< "; Nkvt: " << osu::sum(Nkvt, K, F, T)
						//<< "; Nivt: " << osu::sum(Nivt, N, F, T)
				<< std::endl;
			}
			if (epoch >= options.logfromepoch) {
				if (archiver.outputThetas()) {
					theta = get_theta(Mkt, Alpha, T, N, options, theta);
					archiver.add(theta->data, theta->rows, theta->cols, 0, epoch);
				}
				if (archiver.outputTaus()) {
					tau = get_tau(Nkvt, Beta, T, F, options, tau);
					archiver.add(tau->data, tau->rows, tau->cols, 1, epoch);
				}
				if (archiver.outputPis()) {
					pi = get_pi(Y, Eta, K, N, options, pi);
					archiver.add(pi->data, pi->rows, pi->cols, 2, epoch);
				}
			}
		}
	}

	if (archiver.outputThetas()) {
		theta = get_theta(Mkt, Alpha, T, N, options, theta);
		archiver.add(theta->data, theta->rows, theta->cols, 0, maxEpochs);
	}
	if (archiver.outputTaus()) {
		tau = get_tau(Nkvt, Beta, T, F, options, tau);
		archiver.add(tau->data, tau->rows, tau->cols, 1, maxEpochs);
	}
	if (archiver.outputPis()) {
		pi = get_pi(Y, Eta, K, N, options, pi);
		archiver.add(pi->data, pi->rows, pi->cols, 2, maxEpochs);
	}
	if (archiver.outputGroups()) {
		osu::data::DataMat<int> *groupassgns = new osu::data::DataMat<int>(N, 2);
		for (int i = 0; i < N; i++) {
			groupassgns->data[i][0] = idx2id->at(i);
			groupassgns->data[i][1] = Y[i];
		}
		archiver.add(groupassgns->data, groupassgns->rows, groupassgns->cols, 3, maxEpochs);
		//archiver.add(&Y, N, 1, 3, maxEpochs);
		delete groupassgns;
	}

	if (options.time && timers)
		std::cout << "Times::" << (*timers) << std::endl;

	if (options.debug) {
		//osu::print_arr(Y,N);
	}

	// cleanup
	delete[] Alpha;
	delete[] Beta;
	osu::release_2D_array<int>(Mti, T);
	osu::release_2D_array<int>(Nk, F);
	delete[] Uk;
	delete[] Z;
	delete[] PZ;
	delete[] Y;
	delete[] PY;
	delete[] PYlog;
	delete[] Nw;
	delete[] Ni;
	delete[] Mkt;
	delete[] Nkt;
	delete[] Nit;
	osu::release_3D_array<int>(Nkvt, K, F);
	//osu::release_3D_array<int>(Nivt, N, F);
	delete Nivt;
	delete groupid2idx;
	delete id2idx;
	delete idx2id;
	delete initTimers;
	delete timers;
}

void GroupedNbLDA::populateFixedGroups(int *Y, std::map<int,int> *groupids,
		std::map<int,int> *id2idx, std::map<int,int> *groupid2idx) {
	std::map<int,int>::iterator it = id2idx->begin();
	for (; it != id2idx->end(); it++) {
		int userid = it->first;
		int useridx = it->second;
		int groupid = -1;
		std::map<int,int>::iterator git = groupids->find(userid);
		if (git != groupids->end()) {
			groupid = git->second;
			std::map<int,int>::iterator ggit = groupid2idx->find(groupid);
			if (ggit != groupid2idx->end()) {
				int groupidx = ggit->second;
				Y[useridx] = groupidx;
			}
		}
	}
}

std::map<int,int> *GroupedNbLDA::get_groupid2idx(std::map<int,int> *groupids, osu::Options& options) {
	std::map<int,int> *groupid2idx = new std::map<int,int>();
	std::map<int,int>::iterator it = groupids->begin();
	for (int i = 0; it != groupids->end(); it++) {
		if (groupid2idx->find((*it).second) == groupid2idx->end()) {
			groupid2idx->insert(std::pair<int,int>((*it).second,i));
			i++;
		}
	}
	return groupid2idx;
}

osu::data::DataMat<double> *GroupedNbLDA::getPis() {
	return pi;
}

void GroupedNbLDA::clear_params() {
	if (theta) {
		delete theta;
		theta = 0;
	}
	if (tau) {
		delete tau;
		tau = 0;
	}
	if (pi) {
		delete pi;
		pi = 0;
	}
}

}
}
