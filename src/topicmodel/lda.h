/*
 * lda.h
 *
 *  Created on: Jan 25, 2014
 *      Author: Moy
 */

#ifndef LDA_H_
#define LDA_H_

#include <vector>
#include <map>
#include "../common/utils.h"
#include "../common/Options.h"
#include "../math/math.h"
#include "../data/datamanagement.h"

namespace osu {
namespace lda {

template <typename _V>
std::vector<osu::data::SparseVec<_V>*> *convert_to_sparse(
		osu::data::DataMat<_V> *mat, osu::Options& options) {
	int F = mat->cols;
	int N = mat->rows;
	int tmpIdxs[F]; // = new int[F];
	_V tmpVals[F]; // = new double[F];
	std::vector<osu::data::SparseVec<_V>*> *sparse =
			new std::vector<osu::data::SparseVec<_V>*>();
	for (int i = 0; i < N; i++) {
		int len = 0;
		for (int j = 0; j < F; j++) {
			if (mat->data[i][j] != 0) {
				tmpIdxs[len] = j;
				tmpVals[len] = mat->data[i][j];
				len++;
			}
		}
		int *idxs = new int[len];
		_V *vals = new _V[len];
		std::copy(tmpIdxs,tmpIdxs+len,idxs);
		std::copy(tmpVals,tmpVals+len,vals);
		sparse->push_back(new osu::data::SparseVec<_V>(idxs,vals,len,F));
	}
	return sparse;
}

template <typename _V>
int *get_document_lengths(osu::data::DataMat<_V> *mat,
		osu::Options& options) {
	int F = mat->cols;
	int *doclen = osu::zeros<int>(mat->rows);
	for (int i = 0; i < mat->rows; i++) {
		_V len = 0;
		for (int j = 0; j < F; j++) {
			len += mat->data[i][j];
		}
		doclen[i] = (int)len;
	}
	return doclen;
}

template <typename _V>
int *get_document_lengths(std::vector<osu::data::SparseVec<_V>*> *mat,
		osu::Options& options) {
	if (!mat || mat->size() == 0) return 0;
	int N = mat->size();
	int *doclen = osu::zeros<int>(N);
	typedef std::vector< osu::data::SparseVec<_V>* > InputType;
	typename InputType::iterator it;
	it = mat->begin();
	for (int i = 0; it != mat->end(); it++, i++) {
		osu::data::SparseVec<_V>* fv = *it;
		_V len = osu::sum(fv->vals, fv->len);
		doclen[i] = (int)len;
	}
	return doclen;
}

}
}

#endif /* LDA_H_ */
