#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "spectral_algorithm.h"

//macierz sasiedztwa minus macierz stopni
SparseMatrix* build_laplacian_matrix (SparseMatrix *adj_matrix)
{
   SparseMatrix *degree_matrix = create_degree_matrix(adj_matrix);
   if (!degree_matrix) {
       printf("Błąd: Nie udało się stworzyć macierzy stopni.\n");
       return NULL;
   }
   
   SparseMatrix *laplacian_matrix = (SparseMatrix *)malloc(sizeof(SparseMatrix));
   if (!laplacian_matrix) {
       printf("Błąd: Nie udało się zaalokować pamięci dla macierzy Laplace'a.\n");
       free_sparse_matrix(degree_matrix);
       return NULL;
   }

   laplacian_matrix->rows = adj_matrix->rows;
   laplacian_matrix->cols = adj_matrix->cols;
   laplacian_matrix->nnz = adj_matrix->nnz + degree_matrix->nnz;

   laplacian_matrix->values = (double *)malloc(laplacian_matrix->nnz * sizeof(double));
   laplacian_matrix->col_indices = (int *)malloc(laplacian_matrix->nnz * sizeof(int));
   laplacian_matrix->row_ptr = (int *)malloc((laplacian_matrix->rows + 1) * sizeof(int));

   if (!laplacian_matrix->values || !laplacian_matrix->col_indices || !laplacian_matrix->row_ptr) {
    printf("Błąd: Nie udało się zaalokować pamięci dla elementó macierzy Laplace'a.\n");
    
    free(laplacian_matrix->values);
    free(laplacian_matrix->col_indices);
    free(laplacian_matrix->row_ptr);
    free(laplacian_matrix);
    
    free_sparse_matrix(degree_matrix);
    return NULL;
}
   int nnz_index = 0;
   laplacian_matrix->row_ptr[0] = 0;

   for (int row = 0; row < adj_matrix->rows; row++) {
       int degree = degree_matrix->values[row];

       laplacian_matrix->values[nnz_index] = degree;
       laplacian_matrix->col_indices[nnz_index] = row;
       nnz_index++;

       for (int j = adj_matrix->row_ptr[row]; j < adj_matrix->row_ptr[row + 1]; j++) {
           laplacian_matrix->values[nnz_index] = -adj_matrix->values[j];
           laplacian_matrix->col_indices[nnz_index] = adj_matrix->col_indices[j];
           nnz_index++;
       }

       laplacian_matrix->row_ptr[row + 1] = nnz_index;
   }

   free_sparse_matrix(degree_matrix);
   laplacian_matrix->nnz = nnz_index;
   return laplacian_matrix;
}

DenseVector **compute_eigenvectors(SparseMatrix *laplacian, int num_eigenvectors) {
    if (!laplacian || num_eigenvectors <= 0 || num_eigenvectors > laplacian->rows) {
        printf("Błąd: Niepoprawne dane wejściowe.\n");
        return NULL;
    }

    DenseVector **eigenvectors = (DenseVector **)malloc(num_eigenvectors * sizeof(DenseVector *));
    if (!eigenvectors) {
        printf("Błąd: Nie udało się zaalokować pamięci dla wektorów własnych.\n");
        return NULL;
    }

    for (int i = 0; i < num_eigenvectors; ++i) {
        eigenvectors[i] = (DenseVector *)malloc(sizeof(DenseVector));
        if (!eigenvectors[i]) {
            printf("Błąd: Nie udało się zaalokować pamięci dla wektora własnego: %d.\n", i);
            free_eigenvectors(eigenvectors, i);
            return NULL;
        }

        eigenvectors[i]->size = laplacian->rows;
        eigenvectors[i]->values = (double *)malloc(laplacian->rows * sizeof(double));
        if (!eigenvectors[i]->values) {
            printf("Błąd: Nie udało się zaalokować pamięci dla wartości wektora własnego: %d.\n", i);
            free_eigenvectors(eigenvectors, i);
            return NULL;
        }

        for (int j = 0; j < laplacian->rows; ++j) {
            eigenvectors[i]->values[j] = 2.0 * rand() / RAND_MAX - 1.0;
        }
        normalize_vector(eigenvectors[i]);

        DenseVector *new_vector = (DenseVector *)malloc(sizeof(DenseVector));
        if (!new_vector) {
            free_eigenvectors(eigenvectors, i + 1);
            return NULL;
        }
        new_vector->size = laplacian->rows;
        new_vector->values = (double *)malloc(laplacian->rows * sizeof(double));
        if (!new_vector->values) {
            free(new_vector);
            free_eigenvectors(eigenvectors, i + 1);
            return NULL;
        }

        int max_iterations = 1000;
        double tolerance = 1e-6;
        double prev_eigenvalue = 0.0;

        for (int iter = 0; iter < max_iterations; ++iter) {
            for (int j = 0; j < laplacian->rows; ++j) {
                new_vector->values[j] = eigenvectors[i]->values[j];
            }

            multiply_sparse_matrix_vector(laplacian, new_vector, eigenvectors[i]);

            for (int k = 0; k < i; ++k) {
                double proj = dot_product(eigenvectors[i], eigenvectors[k]);
                for (int j = 0; j < laplacian->rows; ++j) {
                    eigenvectors[i]->values[j] -= proj * eigenvectors[k]->values[j];
                }
            }

            normalize_vector(eigenvectors[i]);

            double eigenvalue = 0.0;
            multiply_sparse_matrix_vector(laplacian, eigenvectors[i], new_vector);
            eigenvalue = dot_product(eigenvectors[i], new_vector);

            if (fabs(eigenvalue - prev_eigenvalue) < tolerance) {
                break;
            }
            prev_eigenvalue = eigenvalue;
        }

        free(new_vector->values);
        free(new_vector);
    }
    return eigenvectors;
}