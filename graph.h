#ifndef GRAPH_H
#define GRAPH_H
#include "io_handler.h"

typedef struct {
    int rows; // Liczba wierszy
    int cols; // Liczba kolumn
    int nnz; // Liczba niezerowych elementow
    double *values; // Wartosci niezerowych elementow
    int *col_indices ; // Indeksy kolumn
    int *row_ptr; // Wskazniki wierszy
} SparseMatrix ;
  
SparseMatrix* create_adjacency_matrix(Graph *graph);
void print_sparse_matrix(SparseMatrix *matrix);
void free_sparse_matrix(SparseMatrix *matrix);
void print_dense_adjacency_matrix(SparseMatrix *matrix);
void print_dense_matrix(SparseMatrix *matrix);

#endif