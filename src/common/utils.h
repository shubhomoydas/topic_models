/*
 * utils.h
 *
 *  Created on: Oct 26, 2013
 *      Author: Moy
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <iomanip>
#include <time.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>

#define APPEND_STR_INT(a,b) osu::strcat<std::string, int>(a,b)

// Interfaces for data load operations

namespace osu {

std::string normalizedFilePath(const std::string& folder, const std::string& file);

// Read a null terminated c_str from binary input stream.
// Assumes that the string in null terminated with a \0 at the end
// of the string in the binary stream. Hence, effectively the string
// read in is 1+length of required string.
std::string read_binary_string(std::ifstream& in);
void write_binary_string(const std::string& str, std::ofstream& out);

template<class T>
void print_arr(T* arr, int len) {
	for (int i = 0; i < len; i++) {
		std::cout << arr[i] << ", ";
	}
	std::cout << std::endl;
}

template<class T>
void print_arr(T** arr, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			std::cout << std::setw(6) << arr[i][j] << ", ";
		}
		std::cout << std::endl;
	}
}

template<class T1, class T2>
std::string strcat(T1 a, T2 b) {
	std::stringstream catstr;
	catstr << a << b;
	return catstr.str();
}

template<class T>
std::string tostring(T a) {
	std::stringstream catstr;
	catstr << a;
	return catstr.str();
}

void trim(std::string& str, const std::string& chars = " \t\n");

void tokenize(const std::string& str,
		std::vector<std::string>& tokens, const std::string& delimiters);

void msg(bool condition, const std::string& message);

template<class T>
void msg(bool condition, const std::string& message, T*** mat, int pages, int rows, int cols) {
	if (!condition) return;
	std::cout << message << std::endl;
	for (int i = 0; i < pages; i++) {
		print_arr(mat[i], rows, cols);
	}
}

template<class T>
void msg(bool condition, const std::string& message, T** mat, int rows, int cols) {
	if (!condition) return;
	std::cout << message << std::endl;
	print_arr(mat, rows, cols);
}

template<class T>
void msg(bool condition, const std::string& message, T* mat, int len) {
	msg(condition, message, &mat, 1, len);
}

template<class T>
void output_data(std::ostream& out, T** data, int rows, int cols) {
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			out << data[i][j] << (j==(cols-1) ? "" : ",");
		}
		out << std::endl;
	}
}

template<class T>
void output_data(std::ostream& out, T** data, int rows, int cols, int idx) {
	for (int i = 0; i < rows; i++) {
		out << idx << ",";
		for (int j = 0; j < cols; j++) {
			out << data[i][j] << (j==(cols-1) ? "" : ",");
		}
		out << std::endl;
	}
}

template<class T>
void output_data(std::ostream& out, T* data, int len, int idx) {
	for (int i = 0; i < len; i++) {
		out << idx << "," << data[i] << std::endl;
	}
}

template<class T>
void release_2D_array(T** arr, int rows) {
	for (int i = 0; i < rows; i++) {
		delete[] arr[i];
	}
	delete[] arr;
}

template<class T>
void release_3D_array(T*** arr, int pages, int rows) {
	for (int i = 0; i < pages; i++) {
		release_2D_array<T>(arr[i], rows);
	}
	delete[] arr;
}

template<typename _V>
void releaseVectorElements(std::vector<_V*>* m) {
	if (!m) return;
	typedef std::vector<_V*> InputType;
	typedef _V ContainerType;
	typename InputType::iterator it;
	for (it = m->begin(); it != m->end(); it++) {
		_V* collection = *it;
		if (collection) delete collection;
	}
}

template<typename _K, typename _V>
void releaseMapElements(std::map<_K,_V>* m) {
	if (!m) return;
	typedef std::map<_K, _V> InputType;
	typename InputType::iterator it;
	for (it = m->begin(); it != m->end(); it++) {
		_V val = it->second;
		if (val) delete val;
	}
}

template<typename _K, typename _V>
void releaseMapElementsWithContainer(std::map<_K,_V*>* m) {
	if (!m) return;
	typedef std::map<_K,_V*> InputType;
	typedef _V ContainerType;
	typename InputType::iterator it;
	for (it = m->begin(); it != m->end(); it++) {
		_V* collection = it->second;
		typename ContainerType::iterator vit;
		if (collection) {
			for (vit = collection->begin(); vit != collection->end(); vit++) {
				delete *vit;
			}
			delete collection;
		}
	}
}

namespace data {

template<class T>
struct DataMat {
public:
	int rows;
	int cols;
	T **data;
	DataMat(): rows(0), cols(0), data(0) {}
	DataMat(int _rows, int _cols): rows(_rows), cols(_cols) {
		data = new T*[rows];
		for (int i = 0; i < rows; i++) {
			data[i] = new T[cols];
		}
	}
	void discardData() {
		if (data) {
			osu::release_2D_array<T>(data, rows);
		}
		rows = 0;
		cols = 0;
		data = 0;
	}
	virtual ~DataMat() {
		discardData();
	}
};

template<class T>
struct SparseVec {
	int *idxs;
	T *vals;
	int len; // the number of features actually present
	int cols; // the total number of features in the dataset
	SparseVec(): idxs(0), vals(0), len(0), cols(0) {};
	SparseVec(int *_idxs, int _len, int _cols):
		idxs(_idxs), vals(0), len(_len), cols(_cols) {}
	SparseVec(int *_idxs, T *_vals, int _len, int _cols):
		idxs(_idxs), vals(_vals), len(_len), cols(_cols) {}
	void add(int idx, T val) {
		int pos = find(idx);
		if (pos > -1) vals[pos] += val;
	}
	void add(osu::data::SparseVec<T> *a, T scale=1) {
		int len_src = a->len;
		int *idxs_src = a->idxs;
		int *vals_src = a->vals;
		int len_dest = len;
		int *idxs_dest = idxs;
		int *vals_dest = vals;
		for (int i = 0, j = 0; i < len_src && j < len_dest;) {
			while (j < len_dest && *idxs_dest < *idxs_src) {
				j++; idxs_dest++; vals_dest++;
			}
			if (j >= len_dest) break;
			while (i < len_src && *idxs_src < *idxs_dest) {
				i++; idxs_src++; vals_src++;
			}
			if (i >= len_src) break;
			if (*idxs_dest == *idxs_src) {
				*vals_dest += (*vals_src * scale);
				j++; idxs_dest++; vals_dest++;
				i++; idxs_src++; vals_src++;
			}
		}
	}
	int find(int idx) {
		int l = 0, r = len;
		while (l < r) {
			int m = (l+r)/2;
			int v = *(idxs+m);
			if (v == idx) return m;
			if (idx < v) r = m; else l = m;
		}
		return -1;
	}
	virtual ~SparseVec() {
		if (idxs) {
			delete[] idxs;
			idxs = 0;
		}
		if (vals) {
			delete[] vals;
			vals = 0;
		}
	}
};

template <typename T>
struct InputData {
public:
	T* data;
	int vocabSize;
	std::map<int,int> *groupids;
	osu::data::DataMat<int> *userids;
	T* holdoutData;
	InputData(T* _data, int _vocabSize,
			std::map<int,int> *_groupids,
			osu::data::DataMat<int> *_userids, T* _holdoutData) :
				data(_data), vocabSize(_vocabSize), groupids(_groupids),
				userids(_userids), holdoutData(_holdoutData) {}
	virtual ~InputData() {
		//
	}
};

struct SparseInputData : public InputData< std::vector<osu::data::SparseVec<int>*> > {
public:
	SparseInputData(std::vector<osu::data::SparseVec<int>*>* _data, int _vocabSize,
			std::map<int,int> *_groupids, osu::data::DataMat<int> *_userids,
			std::vector<osu::data::SparseVec<int>*>* _holdoutData) :
				InputData< std::vector<osu::data::SparseVec<int>*> >(_data, _vocabSize, _groupids, _userids, _holdoutData) {}
	virtual ~SparseInputData() {
		if (data)
			osu::releaseVectorElements< osu::data::SparseVec<int> >(data);
		if (groupids) delete groupids;
		if (userids) delete userids;
		if (holdoutData)
			osu::releaseVectorElements< osu::data::SparseVec<int> >(holdoutData);
	}
};

template <typename T>
struct SparsePagedData {
public:
	int pages;
	int rows;
	int columns;
	osu::data::SparseVec<T> ***data;
	SparsePagedData(int _pages, int _rows, int _columns) :
		pages(_pages), rows(_rows), columns(_columns), data(0) {
		data = new osu::data::SparseVec<T> **[pages];
		for (int i = 0; i < pages; i++) {
			data[i] = new osu::data::SparseVec<T>*[rows];
		}
	}
	osu::data::SparseVec<T>* at(int page, int row) {
		if (page < pages && row < rows) {
			return data[page][row];
		}
		return 0;
	}
	/**
	 * ignores all indexes that are not present in the self and only adds
	 * to those that are present
	 */
	void add(int page, int row, osu::data::SparseVec<T>* a, T scale=1) {
		osu::data::SparseVec<T> *vec = data[page][row];
		vec->add(a, scale);
	}
	/**
	 * The input is a (columns X rows) array
	 */
	void addToArrayT(int page, T **arr, T scale=1) {
		for (int row = 0; row < rows; row++) {
			osu::data::SparseVec<T> *vec = data[page][row];
			int *idxs = vec->idxs;
			int *vals = vec->vals;
			int len = vec->len;
			for (int i = 0; i < len; i++, idxs++, vals++) {
				arr[*idxs][row] += (*vals * scale);
			}
		}
	}
	virtual ~SparsePagedData() {
		if (data) {
			for (int i = 0; i < pages; i++) {
				for (int j = 0; j < rows; j++) {
					delete data[i][j];
				}
				delete[] data[i];
			}
			delete[] data;
		}
	}
};

} // namespace data

} // namespace osu

#endif /* UTILS_H_ */
