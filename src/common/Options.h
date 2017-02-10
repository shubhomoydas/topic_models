/*
 * Options.h
 *
 *  Created on: Oct 26, 2013
 *      Author: Moy
 */

#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <iostream>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string>

using namespace std;

namespace osu {

namespace ConsensusType {
	enum CONSENSUS_TYPE {SIMPLE=0, MEAN=1, MAX=2};
}

class Options {
public:
	std::string data_file;
	std::string vocab_file;
	int rankagg;
	int lda;
	bool sparse;
	bool usegroupids;
	int topics;
	int groups; // Only required for the grouped version of LDA
	int data_dimensions; // = std::numeric_limits<int>::max();
	int skip_top_rows;
	int skip_left_cols;
	int skip_right_cols;
	std::string out_file;
	bool out_to_screen;
	int epochs;
	int numholdout;
	bool randomize;
	uint64_t randseed; // not only input, but also stores the rand seed for saving to model file.
	std::string ranked_file;
	std::string consensus_file;
	int prefix_cols;
	int max_votes;
	int consensus_type;
	double cutoff_fraction;
	std::string thetas_file; // LDA only
	std::string taus_file; // LDA only
	std::string pis_file; // LDA and Plackett-Luce (PL)
	std::string gammas_file; // Plackett-Luce (PL) model parameters only
	std::string groups_file;
	std::string model_file;
	std::string model_folder;
	bool save_model;
	bool load_model;
	bool debug;
	bool time;
	int logfromepoch;
	Options(): data_file(std::string()), vocab_file(std::string()),
			rankagg(1), lda(0), sparse(false),
			usegroupids(false), topics(-1), groups(-1), data_dimensions(100000),
			skip_top_rows(0), skip_left_cols(0), skip_right_cols(0),
			out_file(std::string()), out_to_screen(false), epochs(50), numholdout(0),
			randomize(false), randseed(0), ranked_file(std::string()),
			consensus_file(std::string()), prefix_cols(1), max_votes(50),
			consensus_type(ConsensusType::MEAN), cutoff_fraction(0.50),
			thetas_file(std::string()),
			taus_file(std::string()), pis_file(std::string()),
			gammas_file(std::string()), groups_file(std::string()),
			model_file(std::string()), model_folder(std::string()),
			save_model(false), load_model(false),
			debug(false), time(false), logfromepoch(INT_MAX) {};
	bool parse( int argc, char** argv ) {
		bool returnOptions = true;
	    for (int i = 1; i < argc; ++i) {
			std::string arg = argv[i];
			if ((arg == "-h") || (arg == "-help")) {
				returnOptions = false;
				break;
			} else if ((arg == "-rankagg")) {
				if (i + 1 < argc) {
					rankagg = atoi(argv[++i]);
					if (rankagg < 0 || rankagg > 2) {
						std::cerr << "-rankagg must be one of {0,1,2}." << std::endl;
						rankagg = 0;
					}
				} else {
					std::cerr << "-rankagg option requires one argument {0,1,2}." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-lda")) {
				if (i + 1 < argc) {
					lda = atoi(argv[++i]);
					if (lda < 0 || lda > 3) {
						std::cerr << "-lda must be one of {0,1,2,3}." << std::endl;
						lda = 0;
					}
				} else {
					std::cerr << "-lda option requires one argument {0,1,2,3}." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-T") || (arg == "-topics")) {
				if (i + 1 < argc) {
					topics = atoi(argv[++i]);
				} else {
					std::cerr << "-topics option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-K") || (arg == "-groups")) {
				if (i + 1 < argc) {
					groups = atoi(argv[++i]);
				} else {
					std::cerr << "-groups option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if (arg == "-epochs") {
				if (i + 1 < argc) {
					epochs = atoi(argv[++i]);
				} else {
					std::cerr << "-epochs option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if (arg == "-numholdout") {
				if (i + 1 < argc) {
					numholdout = atoi(argv[++i]);
					if (numholdout < 0) {
						std::cerr << "-numholdout argument must be a positive integer." << std::endl;
						returnOptions = false;
						break;
					}
				} else {
					std::cerr << "-numholdout option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-file")) {
				if (i + 1 < argc) {
					data_file = argv[++i];
				} else {
					std::cerr << "-file option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-vocab")) {
				if (i + 1 < argc) {
					vocab_file = argv[++i];
				} else {
					std::cerr << "-vocab option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-skiptoprows")) {
				if (i + 1 < argc) {
					skip_top_rows = atoi(argv[++i]);
				} else {
					std::cerr << "-skiptoprows option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-skipleftcols")) {
				if (i + 1 < argc) {
					skip_left_cols = atoi(argv[++i]);
				} else {
					std::cerr << "-skipleftcols option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-skiprightcols")) {
				if (i + 1 < argc) {
					skip_right_cols = atoi(argv[++i]);
				} else {
					std::cerr << "-skiprightcols option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-o")) {
				if (i + 1 < argc) {
					out_file = argv[++i];
				} else {
					std::cerr << "-o option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-sparse")) {
				sparse = true;
			} else if ((arg == "-usegroupids")) {
				usegroupids = true;
			} else if ((arg == "-outtoscreen")) {
				out_to_screen = true;
			} else if ((arg == "-time")) {
				time = true;
			} else if ((arg == "-debug")) {
				debug = true;
			} else if ((arg == "-randomize")) {
				randomize = true;
			} else if ((arg == "-rank.file")) {
				if (i + 1 < argc) {
					ranked_file = argv[++i];
				} else {
					std::cerr << "-rank.file option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-rank.consensusfile")) {
				if (i + 1 < argc) {
					consensus_file = argv[++i];
				} else {
					std::cerr << "-rank.consensusfile option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-prefixcols")) {
				if (i + 1 < argc) {
					prefix_cols = atoi(argv[++i]);
				} else {
					std::cerr << "-prefixcols option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if (arg == "-rank.maxvotes") {
				if (i + 1 < argc) {
					max_votes = atoi(argv[++i]);
				} else {
					std::cerr << "-rank.maxvotes option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-rank.consensustype")) {
				if (i + 1 < argc) {
					consensus_type = atoi(argv[++i]);
					if (!(consensus_type == ConsensusType::SIMPLE ||
							consensus_type == ConsensusType::MEAN ||
							consensus_type == ConsensusType::MAX)) {
						consensus_type = ConsensusType::MEAN;
						std::cerr << "-rank.consensustype must be 0 (SIMPLE), 1 (MEAN), or 2 (MAX)" << std::endl;
						returnOptions = false;
						break;
					}
				} else {
					std::cerr << "-rank.consensustype option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if (arg == "-rank.cutofffraction") {
				if (i + 1 < argc) {
					cutoff_fraction = atof(argv[++i]);
					if (cutoff_fraction <= 0 || cutoff_fraction > 1) {
						std::cerr << "-rank.cutofffraction must be in range (0,1]" << std::endl;
						returnOptions = false;
						break;
					}
				} else {
					std::cerr << "-rank.cutoffpct option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-thetasfile")) {
				if (i + 1 < argc) {
					thetas_file = argv[++i];
				} else {
					std::cerr << "-thetasfile option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-tausfile")) {
				if (i + 1 < argc) {
					taus_file = argv[++i];
				} else {
					std::cerr << "-tausfile option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-pisfile")) {
				if (i + 1 < argc) {
					pis_file = argv[++i];
				} else {
					std::cerr << "-pisfile option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-gammasfile")) {
				if (i + 1 < argc) {
					gammas_file = argv[++i];
				} else {
					std::cerr << "-gammasfile option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-groupsfile")) {
				if (i + 1 < argc) {
					groups_file = argv[++i];
				} else {
					std::cerr << "-groupsfile option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-m")) {
				if (i + 1 < argc) {
					model_file = argv[++i];
				} else {
					std::cerr << "-m option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-mf")) {
				if (i + 1 < argc) {
					model_folder = argv[++i];
				} else {
					std::cerr << "-mf option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-savemodel")) {
				save_model = true;
			} else if ((arg == "-loadmodel")) {
				load_model = true;
			} else if ((arg == "-randseed")) {
				if (i + 1 < argc) {
					randseed = atol(argv[++i]);
				} else {
					std::cerr << "-randseed option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else if ((arg == "-logfromepoch")) {
				if (i + 1 < argc) {
					logfromepoch = atoi(argv[++i]);
					if (logfromepoch < 0) {
						std::cerr << "WARN:: -logfromepoch argument must be >= 0." << std::endl;
						logfromepoch = INT_MAX;
					}
				} else {
					std::cerr << "-logfromepoch option requires one argument." << std::endl;
					returnOptions = false;
					break;
				}
			} else {
				std::cerr << "WARN: unrecognized argument '" << arg << "' will be ignored." << std::endl;
			}
	    }
	    if (data_file.empty()) {
	    	std::cerr << "-file <input data file> is mandatory." << std::endl;
	    	returnOptions = false;
	    }
	    if (rankagg > 0) {
	    	if (groups <= 0) {
	    		groups = 1;
	    	}
	    	if (consensus_file.empty()) {
		    	std::cerr << "-rank.consensusfile <consensus_file> is mandatory for ranking." << std::endl;
		    	returnOptions = false;
	    	}
	    }
	    if (sparse && vocab_file.empty()) {
	    	std::cerr << "vocab must be specified for sparse input data." << std::endl;
	    	returnOptions = false;
	    }
	    if (rankagg == 0 && topics <= 0) {
	    	std::cerr << "-topics must specify a positive integer." << std::endl;
	    	returnOptions = false;
	    }
	    if ((save_model || load_model) && model_file.empty()) {
	    	std::cerr << "model_file (-m) must be specified for '-savemodel' and '-loadmodel'." << std::endl;
	    	returnOptions = false;
	    }
	    if (save_model && load_model) {
	    	std::cerr << "Cannot specify both savemodel and loadmodel at the same time." << std::endl;
	    	returnOptions = false;
	    }
	    if (!returnOptions) {
	        showUsage(argv[0]);
	    }
	    return returnOptions;
	}
	void printOptions() {
		std::cout << "data_file: " << data_file << \
				";\nvocab_file: " << vocab_file << \
				";\nrankag: " << rankagg << \
				";\nlda: " << lda << \
				";\nsparse: " << (sparse ? "true" : "false") << \
				";\nusegroupids: " << (usegroupids ? "true" : "false") << \
				";\ntopics: " << topics << \
				";\ngroups: " << groups << \
				"; data_dimensions: " << data_dimensions << \
				";\nskip_top_rows: " << skip_top_rows << \
				"; skip_left_cols: " << skip_left_cols << \
				"; skip_right_cols: " << skip_right_cols << \
				";\nout_file: " << out_file << \
				";\noutputtoscreen: " << (out_to_screen ? "true" : "false") << \
				";\nepochs: " << epochs << \
				";\nnumholdout: " << numholdout << \
				";\nrandomize: " << (randomize ? "true" : "false") << \
				";\nrandseed: " << randseed << \
				";\nranked_file: " << ranked_file << \
				";\nconsensus_file: " << consensus_file << \
				";\nprefix_cols: " << prefix_cols << \
				";\nmax_votes: " << max_votes << \
				";\nconsensus_type: " << (consensus_type == ConsensusType::SIMPLE ? "SIMPLE"
						: (consensus_type == ConsensusType::MEAN ? "MEAN" : "MAX")) << \
				";\ncutoff_fraction: " << cutoff_fraction << \
				";\nthetas_file: " << thetas_file << \
				";\ntaus_file: " << taus_file << \
				";\npis_file: " << pis_file << \
				";\ngammas_file: " << gammas_file << \
				";\ngroups_file: " << groups_file << \
				";\nmodel_file: " << model_file << \
				";\nmodel_folder: " << model_folder << \
				";\nsave_model: " << (save_model ? "true" : "false") << \
				";\nload_model: " << (load_model ? "true" : "false") << \
				";\ntime: " << (time ? "true" : "false") << \
				";\ndebug: " << (debug ? "true" : "false") << \
				";\nlogfromepoch: " << logfromepoch << \
				std::endl;
	}

	void showUsage(std::string name) {
		std::cerr << "use -h to see this help" << std::endl;
		std::cerr << "Usage:" << std::endl;
		std::cerr << ">" << name << \
				" -file <data_file> [-vocab <vocab_file>] [-rankagg <0|1|2>] [-lda <0|1|2>] [-sparse] [-usegroupids] [-topics <#topics>] [-groups <#groups>]" << std::endl << \
				"    [-skiptoprows skip_top_rows] [-skipleftcols skip_left_cols]" << std::endl << \
				"    [-skiprightcols skip_right_cols] [-epochs <#epochs>] [-numholdout <#numholdout>]" << std::endl << \
				"    [-o out_file] [-normalize] [-debug] [-outtoscreen] [-randomize] [-randseed <seed>]" << std::endl << \
				"    [-rank.file <file>] [-rank.consensusfile <file>] [-prefixcols <prefix_cols>] [-rank.maxvotes <maxvotes>]" << std::endl << \
				"    [-rank.consensustype <consensustype>] [-rank.cutofffraction <cutofffraction>]" << std::endl << \
				"    [-thetasfile <thetas_file>] [-tausfile <taus_file>] [-pisfile <pis_file>] [-groupsfile <groups_file>]" << std::endl << \
				"    [-savemodel|-loadmodel] [-m model_file] [-mf model_folder] [-logfromepoch <epoch>]" << std::endl << \
				std::endl;
		std::cerr << "  where:" << std::endl;
		std::cerr << "    data_file: complete path to csv file." << std::endl;
		std::cerr << "    vocab_file: complete path to vocab file. This is required when the input data is sparse." << std::endl;
		std::cerr << "    rankagg: (default 1) The rank aggregation model to use. must be one of {0 - no rank agg, 1 - PL Distribution, 2 - Ranking with SimpleLDA}." << std::endl;
		std::cerr << "    lda: (default 0) The LDA generative model to use. must be one of {0 - SimpleLDA, 1 - MultinomialNBLDA, 2 - GroupedMultinomialNBLDA}." << std::endl;
		std::cerr << "    sparse: (default false) indicates that the file is in sparse format." << std::endl;
		std::cerr << "    usegroupids: (default false) whether the groups present in input data should be kept fixed." << std::endl;
		std::cerr << "    #topics: (>=1) number of topics." << std::endl;
		std::cerr << "    #groups: (>=1) number of groups." << std::endl;
		std::cerr << "    #skip_top_rows: (>=0) number of rows in data file to be skipped." << std::endl;
		std::cerr << "    #skip_left_cols: (>=0) number of start cols in data file to be skipped." << std::endl;
		std::cerr << "    #skip_right_cols: (>=0) number of end cols in data file to be skipped." << std::endl;
		std::cerr << "    out_file: If specified, the anomalies will be written to the output file in CSV format." << std::endl;
		std::cerr << "    outtoscreen: (default false) prints anomalies to screen." << std::endl;
		std::cerr << "    #epochs: (>=1) number of epochs to train." << std::endl;
		std::cerr << "    #numholdout: (>=0, default 0) number of input instances to use as holdout set. The instances will be selected at random." << std::endl;
		std::cerr << "    debug: (default false) prints debug traces during execution when set." << std::endl;
		std::cerr << "    randomize: (default false) use random seed." << std::endl;
		std::cerr << "    randseed: (default 0) random seed." << std::endl;
		std::cerr << "    ranked_file: (optional, default blank) file to save the sorted ranks." << std::endl;
		std::cerr << "    prefix_cols: (optional, default 1) Number of columns from the left in the input file that should be prefixed in the output consensus file." << std::endl;
		std::cerr << "    maxvotes: (optional, default 50) max votes assigned to the top ranked instance." << std::endl;
		std::cerr << "    consensustype: (default 1 (MEAN)) 0 - Simple rank averaging after rank normalization, 1 - Mean of LDA Probs., 2 - Max of LDA Probs." << std::endl;
		std::cerr << "    cutofffraction: (optional, default 0.5) fraction of top ranked instances to use for ranking." << std::endl;
		std::cerr << "    thetas_file: (optional, default blank) file to save the thetas computed." << std::endl;
		std::cerr << "    taus_file: (optional, default blank) file to save the taus computed." << std::endl;
		std::cerr << "    pis_file: (optional, default blank) file to save the pis computed." << std::endl;
		std::cerr << "    gammas_file: (optional, default blank) file to save the gammas computed for PL ranking." << std::endl;
		std::cerr << "    groups_file: (optional, default blank) file to save the groups inferred." << std::endl;
		std::cerr << "    model_file: (default none) file to save the trained model." << std::endl;
		std::cerr << "    model_folder: (optional, default blank) folder relative to which model files are referred." << std::endl;
		std::cerr << "    savemodel: (default false) Whether to save the model to file." << std::endl;
		std::cerr << "    loadmodel: (default false) Whether to load a pre-trained model from file." << std::endl;
		std::cerr << "      NOTE: Specify either savemodel or loadmodel but NOT both." << std::endl;
		std::cerr << "            model_file must be specified for savemodel and loadmodel." << std::endl;
		std::cerr << "    randseed: (default 0) Sets random seed so that results may be replicated between runs.." << std::endl;
		std::cerr << "    logfromepoch: (default INT_MAX) Epoch from which the parameters should be logged." << std::endl;
	}

	virtual ~Options() {};
};

}

#endif /* OPTIONS_H_ */
