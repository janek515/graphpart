#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "matrix_ops.h"

SparseMatrix* transpose_sparse_matrix(SparseMatrix *matrix) {
    //alokowanie pamieci dla macierzy transponowanej
    SparseMatrix *transpose = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    transpose->rows = matrix->cols;
    transpose->cols = matrix->rows;
    transpose->nnz = matrix->nnz;

    transpose->values = (double *)malloc(matrix->nnz * sizeof(double));
    transpose->col_indices = (int *)malloc(matrix->nnz * sizeof(int));
    transpose->row_ptr = (int *)calloc((matrix->cols + 1), sizeof(int));

    for (int i = 0; i < matrix->nnz; i++) {
        transpose->row_ptr[matrix->col_indices[i] + 1]++;
    }

    for (int i = 0; i < matrix->cols; i++) {
        transpose->row_ptr[i + 1] += transpose->row_ptr[i];
    }

    int *current_pos = (int *)calloc(matrix->cols, sizeof(int));
    for (int i = 0; i < matrix->cols; i++) {
        current_pos[i] = transpose->row_ptr[i];
    }

    for (int row = 0; row < matrix->rows; row++) {
        for (int j = matrix->row_ptr[row]; j < matrix->row_ptr[row + 1]; j++) {
            int col = matrix->col_indices[j];
            int pos = current_pos[col];

            transpose->values[pos] = matrix->values[j];
            transpose->col_indices[pos] = row;

            current_pos[col]++;
        }
    }

    free(current_pos);
    return transpose;
}

//funkcja pomocnicza do macierzy sasiedztwa
SparseMatrix* add_sparse_and_transpose_binary(SparseMatrix *matrix) {
    if (!matrix) {
        printf("Błąd: Macierz wejściowa jest NULL.\n");
        return NULL;
    }

    SparseMatrix *transpose = transpose_sparse_matrix(matrix);
    if (!transpose) {
        printf("Błąd: Nie udało się transponować macierzy.\n");
        return NULL;
    }

    SparseMatrix *result = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    if (!result) {
        printf("Błąd: Nie udało się alokować pamięci dla macierzy z wynikiem.\n");
        free_sparse_matrix(transpose);
        return NULL;
    }

    result->rows = matrix->rows;
    result->cols = matrix->cols;
    result->nnz = 0;

    int max_nnz = matrix->nnz + transpose->nnz;
    double *temp_values = (double *)malloc(max_nnz * sizeof(double));
    int *temp_col_indices = (int *)malloc(max_nnz * sizeof(int));
    int *temp_row_ptr = (int *)calloc((matrix->rows + 1), sizeof(int));

    if (!temp_values || !temp_col_indices || !temp_row_ptr) {
        printf("Błąd: Nie udało się alokować pamięci dla tymczasowych mcierzy.\n");
        free(temp_values);
        free(temp_col_indices);
        free(temp_row_ptr);
        free_sparse_matrix(transpose);

        if (result) free(result);
        return NULL;
    }

    int temp_nnz = 0;
    for (int row = 0; row < matrix->rows; row++) {
        int i = matrix->row_ptr[row];
        int j = transpose->row_ptr[row];

        while (i < matrix->row_ptr[row + 1] || j < transpose->row_ptr[row + 1]) {
            int col_i = (i < matrix->row_ptr[row + 1]) ? matrix->col_indices[i] : matrix->cols;
            int col_j = (j < transpose->row_ptr[row + 1]) ? transpose->col_indices[j] : matrix->cols;

            if (col_i == col_j) {
                temp_values[temp_nnz] = 1.0;
                temp_col_indices[temp_nnz] = col_i;
                i++;
                j++;
            } else if (col_i < col_j) {
                temp_values[temp_nnz] = 1.0;
                temp_col_indices[temp_nnz] = col_i;
                i++;
            } else {
                temp_values[temp_nnz] = 1.0;
                temp_col_indices[temp_nnz] = col_j;
                j++;
            }

            temp_nnz++;
        }
        temp_row_ptr[row + 1] = temp_nnz;
    }

    result->values = (double *)malloc(temp_nnz * sizeof(double));
    result->col_indices = (int *)malloc(temp_nnz * sizeof(int));
    result->row_ptr = (int *)malloc((matrix->rows + 1) * sizeof(int));

    if (!result->values || !result->col_indices || !result->row_ptr) {
        printf("Błąd: Nie udało się alokować pamięci dla elementow macierzy wynikowej.\n");
        free(temp_values);
        free(temp_col_indices);
        free(temp_row_ptr);
        free_sparse_matrix(transpose);
        free_sparse_matrix(result);
        return NULL;
    }

    for (int i = 0; i < temp_nnz; i++) {
        result->values[i] = temp_values[i];
        result->col_indices[i] = temp_col_indices[i];
    }
    for (int i = 0; i <= matrix->rows; i++) {
        result->row_ptr[i] = temp_row_ptr[i];
    }
    result->nnz = temp_nnz;

    free(temp_values);
    free(temp_col_indices);
    free(temp_row_ptr);
    free_sparse_matrix(transpose);
    return result;
}

//na podstawie macierzy sasiedzstwa (tej stworzonej z sumy macierzy saiedzstwa i jej transpozycji)!
SparseMatrix* create_degree_matrix(SparseMatrix *adj_matrix) {

    SparseMatrix *degree_matrix = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    degree_matrix->rows = adj_matrix->rows;
    degree_matrix->cols = adj_matrix->cols;
    degree_matrix->nnz = adj_matrix->rows;

    degree_matrix->values = (double *)malloc(degree_matrix->nnz * sizeof(double));
    degree_matrix->col_indices = (int *)malloc(degree_matrix->nnz * sizeof(int));
    degree_matrix->row_ptr = (int *)malloc((degree_matrix->rows + 1) * sizeof(int));

    for (int row = 0; row < adj_matrix->rows; row++) {
        if (row + 1 >= adj_matrix->rows + 1) {
            printf("Błąd: row_ptr przekracza granice.\n");//xd
            return NULL;
        }
        int degree = adj_matrix->row_ptr[row + 1] - adj_matrix->row_ptr[row];

        degree_matrix->values[row] = (double)degree;
        degree_matrix->col_indices[row] = row;
        degree_matrix->row_ptr[row] = row;
    }
    degree_matrix->row_ptr[degree_matrix->rows] = degree_matrix->nnz;
    return degree_matrix;
}

//iloczyn skalarny
double dot_product(DenseVector *v1, DenseVector *v2) {
    double result = 0.0;
    for (int i = 0; i < v1->size; ++i) {
        result += v1->values[i] * v2->values[i];
    }
    return result;
}

//mnozenie macierzy rzadkiej w csr przez wektor gesty
void multiply_sparse_matrix_vector(SparseMatrix *matrix, DenseVector *v, DenseVector *result) {

    for (int i = 0; i < matrix->rows; i++) {
        result->values[i] = 0.0;
        for (int j = matrix->row_ptr[i]; j < matrix->row_ptr[i + 1]; j++) {
            result->values[i] += matrix->values[j] * v->values[matrix->col_indices[j]];
        }
    }
}

//normalizacja wektora
void normalize_vector(DenseVector *v) {
    double norm = 0.0;
    for (int i = 0; i < v->size; ++i) {
        norm += v->values[i] * v->values[i];
    }
    norm = sqrt(norm);

    if (norm == 0.0) {
        printf("Błąd: Norma wektora wynosi 0.\n");
        return; 
    }

    for (int i = 0; i < v->size; ++i) {
        v->values[i] /= norm;
    }
}

void print_eigenvectors(DenseVector **eigenvectors, int num_eigenvectors) {
    if (!eigenvectors || num_eigenvectors <= 0) {
        printf("Błąd: Niepoprawne wektory własne lub liczba wektorów własnych\n");
        return;
    }

    printf("Wektory własne:\n");
    for (int i = 0; i < num_eigenvectors; ++i) {
        if (!eigenvectors[i]) {
            printf("Błąd: Wektor własny %d jest NULL.\n", i + 1);
            continue;
        }

        printf("Wektor własny %d:\n", i + 1);
        for (int j = 0; j < eigenvectors[i]->size; ++j) {
            printf("%8.4f ", eigenvectors[i]->values[j]);
        }
        printf("\n");
    }
}

void free_eigenvectors(DenseVector **eigenvectors, int num_eigenvectors) {
    if (!eigenvectors) return;

    for (int i = 0; i < num_eigenvectors; ++i) {
        if (eigenvectors[i]) {
            free(eigenvectors[i]->values);
            eigenvectors[i]->values = NULL;
            free(eigenvectors[i]);
            eigenvectors[i] = NULL;
        }
    }
    free(eigenvectors);
    eigenvectors = NULL;
}
