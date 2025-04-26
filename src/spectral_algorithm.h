#ifndef SPECTRAL_ALGORITHM_H
#define SPECTRAL_ALGORITHM_H
#include "kmeans.h"
#include "matrix_ops.h"

SparseMatrix *build_laplacian_matrix(SparseMatrix *adj_matrix);
DenseVector **compute_eigenvectors(SparseMatrix *laplacian, int num_eigenvectors);

#endif
