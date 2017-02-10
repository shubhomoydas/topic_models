/*
 * math.cpp
 *
 *  Created on: Oct 27, 2013
 *      Author: Moy
 */

#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include "math.h"
#include "../common/utils.h"

namespace osu {

double unif_rand_01()
{
	return rand() / (double)(RAND_MAX + 1.0);
}

int randint() {
	return 65535*(rand()%65535) + rand()%65535;
}

void generate_uniform_prob_vector(double *probs, int nprobs) {
	for (int i = 0; i < nprobs; i++) {
		probs[i] = unif_rand_01();
	}
	osu::normalize(probs, nprobs);
}

int generate_multinoimal(double *probs, int nprobs) {
	double u = unif_rand_01();
	double curr = 0.0;
	for (int i = 0; i < nprobs; i++) {
		double next = curr + probs[i];
		if (u >= curr && u < next) return i;
		curr = next;
	}
	return -1;
}

void generate_multinoimal_samples(int *arr, int n, double *probs, int nprobs) {
	for (int i = 0; i < n; i++) {
		arr[i] = generate_multinoimal(probs, nprobs);
		if (arr[i] == -1) {
			std::cout << "Fatal Error in probability sampling..." << std::endl;
			osu::print_arr(probs, nprobs);
			exit(-1);
		}
	}
}

int *generate_random_without_replacement(int n, int minRange, int maxRange, bool sorted) {
	if (maxRange < minRange) return 0;
	int range = maxRange-minRange+1;
	if (range < n) return 0;
	int *rnds = new int[n];
	int *tmp = new int[range];
	for (int i = 0; i < range; i++) {
		tmp[i] = i;
	}
	for (int i = 0; i < n; i++, range--) {
		int rnd = randint() % range;
		rnds[i] = minRange + tmp[rnd];
		tmp[rnd] = tmp[range-1];
		//std::cout << "rnd: " << rnd << ", range: " << range << std::endl;
		//osu::print_arr(tmp, range-1);
	}
	if (sorted) std::sort(rnds, rnds+n);
	delete[] tmp;
	return rnds;
}

void set_random_seed(unsigned int seed) {
	srand(seed);
}

//std::uniform_real_distribution<double> unif_01_distribution (0.0,1.0);
//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//std::default_random_engine generator (seed);

/*
	//An online Example for a more complex example
	std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 2);
    for (int n = 0; n < 10; ++n) {
        std::cout << dis(gen) << ' ';
    }
    std::cout << '\n';
*/

}
