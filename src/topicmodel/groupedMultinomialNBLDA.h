/*
 * groupedMultinomialNBLDA.h
 *
 *  Created on: Jan 19, 2014
 *      Author: Moy
 */

#ifndef GROUPEDMULTINOMIALNBLDA_H_
#define GROUPEDMULTINOMIALNBLDA_H_

#include <algorithm>
#include "../math/math.h"
#include "./multinomialNBLDA.h"
namespace osu {
namespace lda {

class GroupedNbLDA : public NbLDA {
public:
//	GroupedNbLDA(osu::data::DataMat<double> *mat,
//			osu::data::DataMat<int> *uids, osu::Options& options)
//	: NbLDA(mat, uids, options) {
//	}
	GroupedNbLDA(std::vector<osu::data::SparseVec<int>*> *_instances,
			std::vector<osu::data::SparseVec<int>*> *_holdout,
			std::map<int,int> *_groupids,
			osu::data::DataMat<int> *_uids, osu::Options& _options) {
		instances = _instances;
		holdout = _holdout;
		uids = _uids;
		useFixedGroupIds = (_options.usegroupids && (_groupids && _groupids->size() > 0));
		groupids = _groupids;
		options = _options;
		if (useFixedGroupIds) {
			std::map<int,int> idxs;
			for (std::map<int,int>::iterator it = groupids->begin();
					it != groupids->end(); it++) {
				idxs.insert(std::pair<int,int>(it->second,it->second));
			}
			options.groups = idxs.size();
			if (options.debug) {
				std::cout << "Setting #groups from input fixed groups: " << idxs.size() << std::endl;
			}
		}
		theta = 0;
		tau = 0;
		pi = 0;
	}
	virtual void runInference();
	virtual osu::data::DataMat<double> *getPis();
	virtual ~GroupedNbLDA() {
		// Since this class does not own this object,
		// DONOT release memory associated with 'mat'.
		instances = 0;
		holdout = 0;
		uids = 0;
		groupids = 0;
		clear_params();
	}

protected:

	// Data structures...
	bool useFixedGroupIds;
	std::map<int,int> *groupids;
	osu::data::DataMat<double> *pi;

	// Methods...

	virtual void clear_params();

	virtual void populateFixedGroups(int *Y, std::map<int,int> *groupids,
			std::map<int,int> *id2idx, std::map<int,int> *groupid2idx);

	virtual std::map<int,int> *get_groupid2idx(std::map<int,int> *groupids,
			osu::Options& options);

	template <typename _V>
	_V *get_user_counts_per_group(
			int *Y, int N,
			osu::Options& options) {
		int K = options.groups;
		_V *Uk = osu::zeros<_V>(K);
		// We MUST assume that the user index ids are in the same order
		// in which they appear in the input matrix
		for (int i = 0; i < N; i++) {
			Uk[Y[i]] += 1;
		}

		// DEBUG...
		//for (int i = 0; i < K; i++) {
		//	std::cout << setw(4) << Uk[i] << " ";
		//}
		//std::cout << std::endl;

		return Uk;
	}
	template <typename _V>
	_V **get_documents_per_group_topic(
			int *Y, int *Z, std::map<int,int> *id2idx, int* Ni, osu::Options& options) {
		int K = options.groups;
		int T = options.topics;
		int N = id2idx->size();
		_V **Mkt = osu::zeros<_V>(K,T);
		// We MUST assume that the user index ids are in the same order
		// in which they appear in the input matrix
		for (int i = 0, m = 0; i < N; i++) {
			int k = Y[i];
			for (int j = 0; j < Ni[i]; j++) {
				Mkt[k][Z[m]] += 1;
				m++;
			}
		}
		return Mkt;
	}
	template <typename _V>
	_V **get_words_per_group_topic(
			int *Y, int *Z, std::map<int,int> *id2idx, int* Ni, int* Nw, osu::Options& options) {
		int K = options.groups;
		int T = options.topics;
		int N = id2idx->size();
		_V **Nkt = osu::zeros<_V>(K,T);
		// We MUST assume that the user index ids are in the same order
		// in which they appear in the input matrix
		for (int i = 0, m = 0; i < N; i++) {
			int k = Y[i];
			for (int j = 0; j < Ni[i]; j++) {
				Nkt[k][Z[m]] += Nw[m];
				m++;
			}
		}
		return Nkt;
	}
	template <typename _V>
	_V **get_words_per_user_topic(
			int *Y, int *Z, std::map<int,int> *id2idx, int* Ni, int* Nw, osu::Options& options) {
		int T = options.topics;
		int N = id2idx->size();
		_V **Nit = osu::zeros<_V>(N,T);
		// We MUST assume that the user index ids are in the same order
		// in which they appear in the input matrix
		for (int i = 0, m = 0; i < N; i++) {
			for (int j = 0; j < Ni[i]; j++) {
				Nit[i][Z[m]] += Nw[m];
				m++;
			}
		}
		return Nit;
	}
	template <typename _V>
	_V ***get_counts_per_group_word_topic(
			std::vector< osu::data::SparseVec<_V>* > *instances,
			int *Y, int *Z, std::map<int,int> *id2idx, int* Ni,
			osu::Options& options) {
		int F = instances->front()->cols;
		int K = options.groups;
		int T = options.topics;
		int N = id2idx->size();
		_V ***Nkvt = new _V**[K];
		for (int k = 0; k < K; k++) {
			Nkvt[k] = osu::zeros<_V>(F,T);
		}
		for (int i = 0, m = 0; i < N; i++) {
			int k = Y[i];
			for (int j = 0; j < Ni[i]; j++, m++) {
				osu::data::SparseVec<_V> *vec = instances->at(m);
				int *idxs = vec->idxs;
				_V  *vals = vec->vals;
				for (int w = 0; w < vec->len; w++, idxs++, vals++) {
					Nkvt[k][*idxs][Z[m]] += *vals;
				}
			}
		}
		return Nkvt;
	}
	template <typename _V>
	_V ***get_counts_per_user_word_topic(
			std::vector< osu::data::SparseVec<_V>* > *instances,
			int *Y, int *Z, std::map<int,int> *id2idx, int* Ni,
			osu::Options& options) {
		int F = instances->front()->cols;
		int T = options.topics;
		int N = id2idx->size();
		_V ***Nivt = new _V**[N];
		for (int i = 0; i < N; i++) {
			Nivt[i] = osu::zeros<_V>(F,T);
		}
		// We MUST assume that the user index ids are in the same order
		// in which they appear in the input matrix
		for (int i = 0, m = 0; i < N; i++) {
			for (int j = 0; j < Ni[i]; j++) {
				osu::data::SparseVec<_V> *vec = instances->at(m);
				int *idxs = vec->idxs;
				_V  *vals = vec->vals;
				for (int w = 0; w < vec->len; w++, idxs++, vals++) {
					Nivt[i][*idxs][Z[m]] += *vals;
				}
				m++;
			}
		}
		return Nivt;
	}

	template <typename _V>
	osu::data::SparsePagedData<_V> *get_sparse_counts_per_user_word_topic(
			std::vector< osu::data::SparseVec<_V>* > *instances,
			int *Y, int *Z, std::map<int,int> *id2idx, int* Ni,
			osu::Options& options) {
		int F = instances->front()->cols;
		int T = options.topics;
		int N = id2idx->size();
		osu::data::SparsePagedData<_V> *Nivt =
				new osu::data::SparsePagedData<_V>(N,T,F);
		osu::data::SparseVec<_V> ***Nivt_data = Nivt->data;
		// We MUST assume that the user index ids are in the same order
		// in which they appear in the input matrix
		std::set<int> idxset;
		int tmpidx[F];
		int sumfeatures = 0;
		for (int i = 0, m = 0; i < N; i++) {
			idxset.clear();
			for (int j = 0; j < Ni[i]; j++) {
				osu::data::SparseVec<_V> *vec = instances->at(m+j);
				int *idxs = vec->idxs;
				int len = vec->len;
				for (int w = 0; w < len; w++, idxs++) {
					idxset.insert(*idxs);
				}
			}
			int nv = idxset.size();
			std::set<int>::iterator it = idxset.begin();
			for (int v = 0; it != idxset.end(); it++, v++) {
				tmpidx[v] = *it;
			}
			std::sort(tmpidx, tmpidx+nv);
			//osu::print_arr(tmpidx, nv);
			for (int t = 0; t < T; t++) {
				int *instidxs = new int[nv];
				std::copy(tmpidx, tmpidx+nv, instidxs);
				_V *instvals = new _V[nv];
				osu::setVal(instvals, nv, 0);
				osu::data::SparseVec<_V> *inst =
						new osu::data::SparseVec<_V>(instidxs, instvals, nv, F);
				Nivt_data[i][t] = inst;
			}
			for (int j = 0; j < Ni[i]; j++) {
				osu::data::SparseVec<_V> *vec = instances->at(m);
				osu::data::SparseVec<_V> *Nivt_it = Nivt_data[i][Z[m]];
				int *idxs = vec->idxs;
				_V  *vals = vec->vals;
				int len = vec->len;
				for (int w = 0; w < len; w++, idxs++, vals++) {
					Nivt_it->add(*idxs, *vals);
				}
				m++;
			}
			sumfeatures += nv;
		}
		if (options.debug) {
			std::cout << "Average features / user: "
					<< ((double)sumfeatures / (double)N) << std::endl;
		}
		return Nivt;
	}

	template <typename _V>
	osu::data::DataMat<double> *get_tau(_V ***Nk,
			double *Beta, int T, int F, osu::Options& options,
			osu::data::DataMat<double> *dest) {
		osu::data::DataMat<double> *Tau = dest;
		if (!Tau) {
			Tau = new osu::data::DataMat<double>(F, T);
			osu::setVal<double>(Tau->data, F, T, 0);
		} else {
			osu::setVal<double>(Tau->data, F, T, 0);
		}
		int K = options.groups;
		double sumBetas = osu::sum(Beta, F);
		for (int t = 0; t < T; t++) {
			for (int k = 0; k < K; k++) {
				osu::col_add<double,_V>(Tau->data, t, Nk[k], t, F);
				osu::col_add<double>(Tau->data, t, Beta, F);
			}
		}
		double *sumRows = new double[T];
		osu::sum_rows(Tau->data, F, T, sumRows);
		for (int t = 0; t < T; t++) {
			osu::col_multiply<double>(Tau->data, t, F, 1.0/(sumBetas+sumRows[t]));
		}
		delete[] sumRows;
		return Tau;
	}

	template <typename _V>
	osu::data::DataMat<double> *get_theta(_V **Nn,
			double *Alpha, int T, int N, osu::Options& options,
			osu::data::DataMat<double> *dest) {
		osu::data::DataMat<double> *Theta = dest;
		int K = options.groups;
		if (!Theta) {
			Theta = new osu::data::DataMat<double>(T, K);
			osu::setVal<double>(Theta->data, T, K, 0);
		} else {
			osu::setVal<double>(Theta->data, T, K, 0);
		}
		double *sumRows = new double[K];
		osu::sum_rows<_V,double>(Nn, T, K, sumRows);
		double sumAlphas = osu::sum(Alpha, T);
		for (int k = 0; k < K; k++) {
			osu::col_add<double,_V>(Theta->data, k, Nn, k, T);
			osu::col_add(Theta->data, k, Alpha, T);
			osu::col_multiply(Theta->data, k, T, 1.0/(sumAlphas+sumRows[k]));
		}
		delete[] sumRows;
		return Theta;
	}

	osu::data::DataMat<double> *get_pi(int *Y,
			double *Eta, int K, int N, osu::Options& options,
			osu::data::DataMat<double> *dest) {
		osu::data::DataMat<double> *Pi = dest;
		if (!Pi) {
			Pi = new osu::data::DataMat<double>(1, K);
			osu::setVal<double>(Pi->data, 1, K, 0);
		} else {
			osu::setVal<double>(Pi->data, 1, K, 0);
		}
		for (int i = 0; i < N; i++) {
			Pi->data[0][Y[i]] = Pi->data[0][Y[i]] + 1;
		}
		for (int k = 0; k < K; k++) {
			Pi->data[0][k] = Pi->data[0][k] / N;
		}
		return Pi;
	}

};

}
}

#endif /* GROUPEDMULTINOMIALNBLDA_H_ */
