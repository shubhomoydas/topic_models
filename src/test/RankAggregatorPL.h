/*
 * RankAggregatorPL.h
 *
 *  Created on: Jun 21, 2014
 *      Author: Moy
 */

#ifndef RANKAGGREGATORPL_H_
#define RANKAGGREGATORPL_H_

#include "RankAggregator.h"

namespace osu {

/**
 * Performs rank aggregation using Plackett-Luce (PL) model
 */
class RankAggregatorPL: public osu::RankAggregator {
public:
	RankAggregatorPL(osu::Options& _options): options(_options),
			mat(0), uids(0), firstCols(0) {}
	virtual ~RankAggregatorPL();
	virtual void loadData();
	virtual void rank();
protected:
	virtual void rankSimple();
	virtual void rankLDA();
private:
	void loadRankedData();
	void computeCumsum(int **S, double **Gs, double **Gs_sum, int k, int d, int N);
	double computePLLogLik(int **S, double **Gs, double *Pi,
			double **Z, int K, int D, int N );
	void output_to_file(std::vector< std::pair<int, double>* > *data,
			std::vector<std::string> *pre, std::ofstream& out);
	osu::Options options;
	osu::data::DataMat<int> *mat;
	osu::data::DataMat<int> *uids;
	std::vector<std::string> *firstCols;
};

}

#endif /* RANKAGGREGATORPL_H_ */
