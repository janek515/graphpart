#ifndef MATRIX_OPS_H
#define MATRIX_OPS_H
#include "graph.h"

SparseMatrix *
transpose_sparse_matrix(SparseMatrix *matrix); // fcja pomocnicza do macierzy sasiedztwa
SparseMatrix *add_sparse_and_transpose_binary(SparseMatrix *matrix); // stad macierz sasiedztwa
SparseMatrix *
create_degree_matrix(SparseMatrix *adj_matrix); // na podstawie macierzy sasiedzstwa (tej stworzonej
                                                // z sumy macierzy saiedzstwa i jej transpozycji)!

typedef struct {
    int size;       // Rozmiar wektora
    double *values; // Wartosci wektora
} DenseVector;

double dot_product(DenseVector *v1, DenseVector *v2);
void multiply_sparse_matrix_vector(SparseMatrix *matrix, DenseVector *v, DenseVector *result);
void normalize_vector(DenseVector *v);
void print_eigenvectors(DenseVector **eigenvectors, int num_eigenvectors);
void free_eigenvectors(DenseVector **eigenvectors, int num_eigenvectors);

#endif
