/*
 * RankAggregator.h
 *
 *  Created on: Jan 17, 2014
 *      Author: Moy
 */

#ifndef RANKAGGREGATOR_H_
#define RANKAGGREGATOR_H_

#include <algorithm>
#include "../common/utils.h"
#include "../common/Options.h"
#include "../data/datamanagement.h"

namespace osu {

class RankAggregator {
public:
	RankAggregator(): mat(0), uids(0), firstCols(0) {}
	RankAggregator(osu::Options& _options): options(_options),
			mat(0), uids(0), firstCols(0) {}
	virtual void loadData();
	virtual void rank();
	virtual ~RankAggregator();

protected:

	template <typename T, typename K>
	struct compAlgo {
		// compares in descending order
		bool operator() (std::pair<T,K> *i, std::pair<T,K> *j) {
			return (i->second > j->second);
		}
	};

	template <typename T>
	osu::data::DataMat<T> *transpose(osu::data::DataMat<T> *mat) {
		osu::data::DataMat<T> *t = new osu::data::DataMat<T>(mat->cols, mat->rows);
		for (int i = 0; i < mat->rows; i++) {
			T *src = mat->data[i];
			for (int j = 0; j < mat->cols; j++, src++) {
				t->data[j][i] = *src;
			}
		}
		return t;
	}

	template <typename T>
	T **transpose(T **data, int rows, int cols) {
		T **t = new T*[cols];
		for (int i = 0; i < cols; i++) {
			t[i] = new T[rows];
		}
		for (int i = 0; i < rows; i++) {
			T *src = data[i];
			for (int j = 0; j < cols; j++, src++) {
				t[j][i] = *src;
			}
		}
		return t;
	}

	virtual void normalizeRanks(osu::data::DataMat<int> *mat, osu::Options &options);
	virtual std::vector< std::pair<int, double>* > *applyConsensus(
			osu::data::DataMat<double> *thetas,
			osu::data::DataMat<double> *taus);
	virtual void rankSimple();
	virtual void rankLDA();

	/**
	 * Assigns ranks to each instance
	 */
	template <typename T>
	osu::data::DataMat<int> *sortAndRank(osu::data::DataMat<T> *mat) {
		osu::data::DataMat<int> *ranked =
				new osu::data::DataMat<int>(mat->rows, mat->cols);
		std::vector< std::pair<int, T>* > *tmp =
				new std::vector< std::pair<int, T>* >();
		for (int j = 0; j < mat->cols; j++) {
			tmp->push_back(new std::pair<int, T>(0,0));
		}
		compAlgo<int,T> CompAlgo;
		for (int i = 0; i < mat->rows; i++) {
			T *src = mat->data[i];
			for (int j = 0; j < mat->cols; j++, src++) {
				(tmp->at(j))->first = j;
				(tmp->at(j))->second = *src;
			}
			std::sort(tmp->begin(), tmp->end(), CompAlgo);
			int *dest = ranked->data[i];
			for (int j = 0; j < ranked->cols; j++) {
				dest[(tmp->at(j))->first] = j+1;
			}
		}
		osu::releaseVectorElements< std::pair<int, T> >(tmp);
		return ranked;
	}

	/**
	 * sorts the instance scores and then places their corresponding
	 * ids in the order of scores.
	 */
	template <typename T>
	osu::data::DataMat<int> *sortAndOrder(osu::data::DataMat<T> *mat) {
		osu::data::DataMat<int> *ranked =
				new osu::data::DataMat<int>(mat->rows, mat->cols);
		std::vector< std::pair<int, T>* > *tmp =
				new std::vector< std::pair<int, T>* >();
		for (int j = 0; j < mat->cols; j++) {
			tmp->push_back(new std::pair<int, T>(0,0));
		}
		compAlgo<int,T> CompAlgo;
		for (int i = 0; i < mat->rows; i++) {
			T *src = mat->data[i];
			for (int j = 0; j < mat->cols; j++, src++) {
				(tmp->at(j))->first = j;
				(tmp->at(j))->second = *src;
			}
			std::sort(tmp->begin(), tmp->end(), CompAlgo);
			int *dest = ranked->data[i];
			for (int j = 0; j < ranked->cols; j++) {
				dest[j] = (tmp->at(j))->first+1;
			}
		}
		osu::releaseVectorElements< std::pair<int, T> >(tmp);
		return ranked;
	}

private:
	osu::Options options;
	osu::data::DataMat<int> *mat;
	osu::data::DataMat<int> *uids;
	std::vector<std::string> *firstCols;
};

}

#endif /* RANKAGGREGATOR_H_ */
