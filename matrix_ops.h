#ifndef MATRIX_OPS_H
#define MATRIX_OPS_H
#include "graph.h"

SparseMatrix* transpose_sparse_matrix(SparseMatrix *matrix);
SparseMatrix* add_sparse_and_transpose_binary(SparseMatrix *matrix);
SparseMatrix* create_degree_matrix(SparseMatrix *adj_matrix);

#endif