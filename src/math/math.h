/*
 * math.h
 *
 *  Created on: Oct 27, 2013
 *      Author: Moy
 */

#ifndef MATH_H_
#define MATH_H_

#include <cmath>

namespace osu {

/**
 * Generates a uniform random number in range [0.0,1.0]
 */
double unif_rand_01();

/**
 * Generates a long random integer. The rand() function usually only
 * returns a value in the range 0-35567. We sometimes need larger values.
 */
int randint();

void generate_uniform_prob_vector(double *probs, int nprobs);

/**
 * Generates a sample from a discrete multinomial distribution
 * based on the probabilities provided in the matrix.
 * No checks are performed to ensure that the probabilities sum to one.
 */
int generate_multinoimal(double *probs, int nprobs);

void generate_multinoimal_samples(int *arr, int n, double *probs, int nprobs);

/**
 * Both maxRange and minRange are inclusive
 */
int *generate_random_without_replacement(int n, int minRange, int maxRange, bool sorted=false);

void set_random_seed(unsigned int seed);

template<class T>
inline void setVal(T* arr, int len, T val) {
	std::fill_n(arr,len,val);
}

template<class T>
inline void setVal(T** arr, int rows, int cols, T val) {
	for (int i = 0; i < rows; i++) {
		std::fill_n(arr[i],cols,val);
	}
}

template<class T>
inline void setVal(T*** arr, int pages, int rows, int cols, T val) {
	for (int page = 0; page < pages; page++) {
		setVal(arr[page], rows, cols, val);
	}
}

/**
 * MATLAB:
 * A(:,col) = A(:,col) + B*scale
 */
template<class T>
inline void col_add(T** a, int col, T* b, int rows, T scale = 1) {
	for (int i = 0; i < rows; i++) {
		a[i][col] = a[i][col] + b[i]*scale;
	}
}

/**
 * MATLAB:
 * Dest(:,:) = Src(:,:) * scale
 */
template<class T>
inline void copy(T** src, T** dest, int rows, int cols, T scale = 1) {
	T **src_a = src, **dest_b = dest;
	for (int i = 0; i < rows; i++, src_a++, dest_b++) {
		T* src_pos = *src_a;
		T* dest_pos = *dest_b;
		for (int j = 0; j < cols; j++, dest_pos++, src_pos++) {
			*dest_pos = (*src_pos)*scale;
		}
	}
}

template<class K,class T>
inline void col_add(K** a, int a_col, T** b, int b_col, int rows, K scale = 1) {
	for (int i = 0; i < rows; i++) {
		a[i][a_col] = a[i][a_col] + ((K)b[i][b_col])*scale;
	}
}

template<class K,class T>
inline void col_add(K** a, int col, T* b, int *idxs, int idxslen, K scale = 1) {
	int *ptr = idxs;
	for (int i = 0; i < idxslen; i++, ptr++) {
		a[*ptr][col] = a[*ptr][col] + ((K)b[*ptr])*scale;
	}
}

template<class K,class T>
inline void col_add_vals_idxs(K** a, int col, T* vals, int *idxs, int idxslen, K scale = 1) {
	int *iPtr = idxs;
	int *vPtr = vals;
	for (int i = 0; i < idxslen; i++, iPtr++, vPtr++) {
		a[*iPtr][col] = a[*iPtr][col] + ((K)*vPtr)*scale;
	}
}

/**
 * MATLAB:
 * A(:,col) = A(:,col) + B(row,:)'*scale
 */
template<class T>
inline void add_col_row(T** a, int col, T** b, int row, int len, T scale = 1) {
	int *b_ptr = b[row];
	for (int i = 0; i < len; i++) {
		a[i][col] = a[i][col] + b_ptr[i]*scale;
	}
}

/**
 * MATLAB:
 * A(row,:) = A(row,:) + B(:,col)'*scale
 */
template<class T>
inline void add_row_col(T** a, int row, T** b, int col, int len, T scale = 1) {
	T *dest = a[row];
	for (int i = 0; i < len; i++, dest++) {
		*dest = *dest + b[i][col]*scale;
	}
}

/**
 * MATLAB:
 * dest = A(row,:) + B(row,:)*scale
 */
template<class K,class S,class T>
inline void add_rows(K** a, int row_a, S** b, int row_b, int len, T *dest, T scale = 1) {
	K *src_a = a[row_a];
	S *src_b = b[row_b];
	T *dest_ptr = dest;
	for (int i = 0; i < len; i++, dest_ptr++, src_a++, src_b++) {
		*dest_ptr = (T)(*src_a) + ((T)(*src_b))*scale;
	}
}

/**
 * MATLAB:
 * A = A + val
 */
template<class T>
inline void add(T* a, int len, T val) {
	T *dest = a;
	for (int i = 0; i < len; i++, dest++) {
		*dest = *dest + val;
	}
}

/**
 * MATLAB:
 * A = A + B*scale
 */
template<class T>
inline void add(T** a, T** b, int rows, int cols, T scale = 1) {
	T **src_a = a, **src_b = b;
	for (int i = 0; i < rows; i++, src_a++, src_b++) {
		T* dest = *src_a;
		T* src = *src_b;
		for (int j = 0; j < cols; j++, dest++, src++) {
			*dest = *dest + (*src)*scale;
		}
	}
}

/**
 * MATLAB:
 * dest = dest + a * a'; % a is a column vector, dest is a square matrix
 */
template<class T>
inline void add_a_a_transpose(T* a, int len, T** dest) {
	T **dest_ptr_r = dest;
	T *src_ptr_i = a;
	for (int i = 0; i < len; i++, dest_ptr_r++, src_ptr_i++) {
		T *src_ptr_j = a;
		T *dest_ptr_c = *dest_ptr_r;
		for (int j = 0; j < len; j++, dest_ptr_c++, src_ptr_j++) {
			*dest_ptr_c = (*dest_ptr_c) + (*src_ptr_i)*(*src_ptr_j);
		}
	}
}

template<class T>
inline void col_multiply(T** a, int col, T* b, int rows) {
	for (int i = 0; i < rows; i++) {
		a[i][col] = a[i][col] * b[i];
	}
}

template<class T>
inline void col_multiply(T** a, int col, int rows, T val) {
	for (int i = 0; i < rows; i++) {
		a[i][col] = a[i][col] * val;
	}
}

template<class T>
inline void col_multiply(T** a, int col, T* b, int *idxs, int idxslen) {
	int *ptr = idxs;
	for (int i = 0; i < idxslen; i++, ptr++) {
		a[*ptr][col] = a[*ptr][col] * b[*ptr];
	}
}

template<class T>
inline T col_sum(T** a, int rows, int col) {
	T s = 0;
	for (int i = 0; i < rows; i++) {
		s += a[i][col];
	}
	return s;
}

template<class T>
inline T row_sum(T** a, int row, int cols) {
	T s = 0;
	T *src = a[row];
	for (int i = 0; i < cols; i++, src++) {
		s += (*src);
	}
	return s;
}

template<class T>
inline void exp(T* data, int len) {
	T *dest = data;
	for (int i = 0; i < len; i++, dest++) {
		*dest = std::exp(*dest);
	}
}

template<class T>
inline void mult_array(T* data, int len, T val, T* dest = 0) {
	T *destination = dest;
	T *src = data;
	if (!destination) destination = data;
	for (int i = 0; i < len; i++, destination++, src++) {
		*destination = (*src)*val;
	}
}

template<class T>
inline T min(T* data, int len) {
	T s = len > 0 ? data[0] : 0;
	T *src = data;
	for (int i = 0; i < len; i++, src++) {
		s = (*src < s ? *src : s);
	}
	return s;
}

template<class T>
inline void min(T* data, T cmpVal, int len, T* dest=0) {
	T *result = (dest ? dest : data);
	T *src = data;
	for (int i = 0; i < len; i++, src++, result++) {
		*result = (cmpVal < *src ? cmpVal : *src);
	}
}

template<class T>
inline T max(T* data, int len) {
	T s = len > 0 ? data[0] : 0;
	T *src = data;
	for (int i = 0; i < len; i++, src++) {
		s = (*src > s ? *src : s);
	}
	return s;
}

template<class T>
inline void max(T* data, T cmpVal, int len, T* dest=0) {
	T *result = (dest ? dest : data);
	T *src = data;
	for (int i = 0; i < len; i++, src++, result++) {
		*result = (cmpVal > *src ? cmpVal : *src);
	}
}

template<class T>
inline void invert_elements(T* data, int len, T* dest=0) {
	T* result = (dest ? dest : data);
	for (int i = 0; i < len; i++) {
		result[i] = 1.0/data[i];
	}
}

template<class T>
inline void normalize(T* data, int len) {
	T sum = (T)0;
	T *dest = data;
	for (int i = 0; i < len; i++, dest++) sum += (*dest);
	dest = data;
	if ((double)sum != 0.0) {
		for (int i = 0; i < len; i++, dest++) *dest = (*dest)/sum;
	}
}

template<class T>
inline T sum(T* data, int len) {
	T s = 0;
	T *src = data;
	for (int i = 0; i < len; i++, src++) {
		s += (*src);
	}
	return s;
}

template<class T>
inline T sum(T** data, int rows, int cols) {
	T sum = 0;
	for (int i = 0; i < rows; i++) {
		T *src = data[i];
		for (int j = 0; j < cols; j++, src++)
			sum += (*src);
	}
	return sum;
}

template<class T>
inline T sum(T*** data, int pages, int rows, int cols) {
	T sum = 0;
	T ***src_pg = data;
	for (int k = 0; k < pages; k++, src_pg++) {
		T **src_r = *src_pg;
		for (int i = 0; i < rows; i++, src_r++) {
			T *src_c = *src_r;
			for (int j = 0; j < cols; j++, src_c++)
				sum += (*src_c);
		}
	}
	return sum;
}

template<class T>
inline T* sum_cols(T** data, int rows, int cols, T* dest = 0) {
	T* sums = dest;
	if (!sums) sums = new double[rows];
	osu::setVal<double>(sums, rows, 0);
	for (int i = 0; i < rows; i++) {
		T sum = 0;
		T *src = data[i];
		for (int j = 0; j < cols; j++, src++) sum += (*src);
		sums[i] = sum;
	}
	return sums;
}

template<class K,class T>
inline T* sum_rows(K** data, int rows, int cols, T* dest = 0) {
	T* sums = dest;
	if (!sums) sums = new T[cols];
	osu::setVal<T>(sums, cols, 0);
	for (int j = 0; j < cols; j++) {
		T sum = 0;
		for (int i = 0; i < rows; i++) sum += (T)data[i][j];
		sums[j] = sum;
	}
	return sums;
}

template<class K,class T>
inline T* sum_row_range(K** data, int start_row, int rows, int cols, T* dest = 0) {
	T* sums = dest;
	if (!sums) sums = new T[cols];
	osu::setVal<T>(sums, cols, 0);
	for (int j = 0; j < cols; j++) {
		T sum = 0;
		for (int i = 0; i < rows; i++) sum += (T)data[start_row+i][j];
		sums[j] = sum;
	}
	return sums;
}

template<class T>
inline T* zeros(int len) {
	T* arr = new T[len];
	std::fill_n(arr,len,0);
	return arr;
}

template<class T>
inline T** zeros(int rows, int cols) {
	T** arr = new T*[rows];
	for (int i = 0; i < rows; i++) {
		arr[i] = new T[cols];
		std::fill_n(arr[i],cols,0);
	}
	return arr;
}

template<class T>
inline T*** zeros(int pages, int rows, int cols) {
	T*** arr = new T**[pages];
	for (int k = 0; k < pages; k++) {
		arr[k] = zeros<T>(rows, cols);
	}
	return arr;
}

template<class T>
inline T* ones(int len) {
	T* arr = new T[len];
	std::fill_n(arr,len,1);
	return arr;
}

template<class T>
inline T** ones(int rows, int cols) {
	T** arr = new T*[rows];
	for (int i = 0; i < rows; i++) {
		arr[i] = new T[cols];
		std::fill_n(arr[i],cols,1);
	}
	return arr;
}

}

#endif /* MATH_H_ */
