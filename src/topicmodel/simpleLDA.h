/*
 * simpleLDA.h
 *
 *  Created on: Jan 25, 2014
 *      Author: Moy
 */

#ifndef SIMPLELDA_H_
#define SIMPLELDA_H_

#include "lda.h"

namespace osu {
namespace lda {

class SimpleLDA {
public:
	SimpleLDA(std::vector<osu::data::SparseVec<int>*> *_instances,
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
	virtual double getPerplexity(std::vector<osu::data::SparseVec<int>*> *testInstances);
	virtual ~SimpleLDA() {
		// Since this class does not own this object,
		// DONOT release memory associated with 'mat'.
		//osu::releaseVectorElements< osu::data::SparseVec<int> >(instances);
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

	virtual void clear_params();

	/**
	 * For use with sparse vectors
	 */
	template <typename _V>
	int **get_word_counts_per_topic(
			std::vector<osu::data::SparseVec<_V>*> *mat,
			int *Z, osu::Options& options) {
		int F = (mat->front())->cols;
		int T = options.topics;
		int **Nk = osu::zeros<int>(F,T);
		int *zptr = Z;
		typedef std::vector< osu::data::SparseVec<_V>* > InputType;
		typename InputType::iterator it;
		it = mat->begin();
		for (it = mat->begin(); it != mat->end(); it++) {
			osu::data::SparseVec<_V>* fv = *it;
			for (int f = 0; f < fv->len; f++) {
				int v  = fv->idxs[f];
				int nw = fv->vals[f];
				for (int w = 0; w < nw; w++) {
					Nk[v][*zptr] = Nk[v][*zptr] + 1;
					zptr++;
				}
			}
		}
		return Nk;
	}

	template <typename _V>
	int **get_topic_counts_per_document(
			std::vector<osu::data::SparseVec<_V>*> *mat,
			int *Z, osu::Options& options) {
		int T = options.topics;
		int N = mat->size();
		int **Nm = osu::zeros<int>(T, N);
		int *zptr = Z;
		typedef std::vector< osu::data::SparseVec<_V>* > InputType;
		typename InputType::iterator it;
		it = mat->begin();
		for (int i = 0; it != mat->end(); it++, i++) {
			osu::data::SparseVec<_V>* fv = *it;
			for (int f = 0; f < fv->len; f++) {
				int nw = fv->vals[f];
				for (int w = 0; w < nw; w++) {
					Nm[*zptr][i] = Nm[*zptr][i] + 1;
					zptr++;
				}
			}
		}
		return Nm;
	}

	/**
	 * For use with dense vectors
	 */
	virtual int **get_word_counts_per_topic(
			osu::data::DataMat<int> *mat, int *Z, osu::Options& options);

	virtual int **get_topic_counts_per_document(
			osu::data::DataMat<int> *mat, int *Z, osu::Options& options);

	virtual osu::data::DataMat<double> *get_tau(int ***Nk,
			double *Beta, int T, int F, osu::Options& options,
			osu::data::DataMat<double> *dest = 0);
	virtual osu::data::DataMat<double> *get_theta(int **Nn,
			double *Alpha, int T, int N, osu::Options& options,
			osu::data::DataMat<double> *dest = 0);

private:

	template<typename T>
	void print_column(T** arr, int rows, int col) {
		for (int i = 0; i < rows; i++) {
			std::cout << arr[i][col] << " ";
		}
		std::cout << endl;
	}

};

}
}

#endif /* SIMPLELDA_H_ */
