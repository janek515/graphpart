#ifndef SPECTRAL_ALGORITHM_H
#define SPECTRAL_ALGORITHM_H
#include "matrix_ops.h"
#include "kmeans.h"

SparseMatrix* build_laplacian_matrix (SparseMatrix *adj_matrix);
DenseVector **compute_eigenvectors(SparseMatrix *laplacian, int num_eigenvectors);

#endif