/*
 * multinomialNBLDA.h
 *
 *  Created on: Oct 28, 2013
 *      Author: Moy
 */

#ifndef MULTINOMIALNBLDA_H_
#define MULTINOMIALNBLDA_H_

#include "lda.h"

namespace osu {
namespace lda {

class NbLDA {
public:
	NbLDA() {
		instances = 0;
		holdout = 0;
		uids = 0;
		theta = 0;
		tau = 0;
	}
	NbLDA(std::vector<osu::data::SparseVec<int>*> *_instances,
			std::vector<osu::data::SparseVec<int>*> *_holdout,
			osu::data::DataMat<int> *_uids, osu::Options& _options) {
		instances = _instances;
		holdout = _holdout;
		uids = _uids;
		options = _options;
		theta = 0;
		tau = 0;
	}
	virtual void runInference();
	virtual osu::data::DataMat<double> *getThetas();
	virtual osu::data::DataMat<double> *getTaus();
	virtual ~NbLDA() {
		// Since this class does not own this object,
		// DONOT release memory associated with 'mat'.
		instances = 0;
		holdout = 0;
		uids = 0;
		clear_params();
	}

protected:

	// Data structures...
	osu::Options options;
	std::vector<osu::data::SparseVec<int>*> *instances;
	std::vector<osu::data::SparseVec<int>*> *holdout;
	osu::data::DataMat<int> *uids;

	osu::data::DataMat<double> *theta;
	osu::data::DataMat<double> *tau;

	// Methods...

	/**
	 * This is different from the case of SimpleLDA since here we
	 * assume that the topics are for a document whereas SimpleLDA
	 * assumes that topics are for each word.
	 */
	template <typename _V>
	_V **get_word_counts_per_topic(std::vector<osu::data::SparseVec<_V>*> *mat,
			int *Z, osu::Options& options) {
		int F = (mat->front())->cols;
		int T = options.topics;
		int N = mat->size();
		_V **Nk = osu::zeros<_V>(F,T);
		typedef std::vector< osu::data::SparseVec<_V>* > InputType;
		typename InputType::iterator it;
		it = mat->begin();
		for (int i = 0; i < N; it++, i++) {
			osu::data::SparseVec<_V>* fv = *it;
			osu::col_add_vals_idxs<_V>(Nk, Z[i], fv->vals, fv->idxs, fv->len, 1);
		}
		return Nk;
	}
	template <typename _V>
	_V **get_word_counts_per_topic(osu::data::DataMat<_V> *mat, int *Z,
			osu::Options& options) {
		int F = mat->cols;
		int T = options.topics;
		int N = mat->rows;
		_V **Nk = osu::zeros<_V>(F,T);
		for (int i = 0; i < N; i++) {
			_V *fv = mat->data[i];
			osu::col_add<_V>(Nk, Z[i], fv, F, 1);
		}
		return Nk;
	}

	virtual std::map<int,int> *get_id2idx(
			osu::data::DataMat<int> *uids, osu::Options& options);
	virtual std::map<int,int> *get_idx2id(
			std::map<int,int> *id2idx, osu::Options& options);

	virtual osu::data::DataMat<double> *get_tau(int ***Nk,
			double *Beta, int T, int F, osu::Options& options,
			osu::data::DataMat<double> *dest = 0);
	virtual osu::data::DataMat<double> *get_theta(int **Nn,
			double *Alpha, int T, int N, osu::Options& options,
			osu::data::DataMat<double> *dest = 0);

	virtual void clear_params();

	template <typename _V>
	_V **get_topic_counts_per_user(
			osu::data::DataMat<int> *uids, int *Z,
			std::map<int,int> *id2idx, osu::Options& options) {
		int T = options.topics;
		int N = id2idx->size();
		_V **Nn = osu::zeros<_V>(T,N);
		for (int i = 0; i < uids->cols; i++) {
			int idx = id2idx->find(uids->data[0][i])->second;
			//std::cout << "User Id: " << uidx << "Topic: " << Z[i] << std::endl;
			Nn[Z[i]][idx]++;
		}
		return Nn;
	}

	template <typename _V>
	int *get_per_user_doc_counts(osu::data::DataMat<int> *uids,
			std::map<int,int> *id2idx, osu::Options& options) {
		int *doccounts = new int[id2idx->size()];
		osu::setVal<int>(doccounts, id2idx->size(), 0);
		for (int i = 0; i < uids->cols; i++) {
			int id = uids->data[0][i];
			int idx = id2idx->at(id);
			doccounts[idx] += 1;
		}
	//	for (std::map<int,int>::iterator itr = id2idx->begin();
	//			itr != id2idx->end(); itr++) {
	//		std::cout << itr->first << ": " << doccounts[itr->second] << std::endl;
	//	}
		return doccounts;
	}

};

}
}

#endif /* MULTINOMIALNBLDA_H_ */
