#include <stdlib.h>
#include <stdio.h>
#include "graph.h"

SparseMatrix* transpose_sparse_matrix(SparseMatrix *matrix) {
    // Allocate memory for the transposed matrix
    SparseMatrix *transpose = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    transpose->rows = matrix->cols;
    transpose->cols = matrix->rows;
    transpose->nnz = matrix->nnz;

    // Allocate memory for the transposed matrix arrays
    transpose->values = (double *)malloc(matrix->nnz * sizeof(double));
    transpose->col_indices = (int *)malloc(matrix->nnz * sizeof(int));
    transpose->row_ptr = (int *)calloc((matrix->cols + 1), sizeof(int));

    // Step 1: Count the number of non-zero elements in each column of the original matrix
    for (int i = 0; i < matrix->nnz; i++) {
        transpose->row_ptr[matrix->col_indices[i] + 1]++;
    }

    // Step 2: Compute the cumulative sum to get the row_ptr array
    for (int i = 0; i < matrix->cols; i++) {
        transpose->row_ptr[i + 1] += transpose->row_ptr[i];
    }

    // Step 3: Populate the transposed matrix
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

    // Free temporary memory
    free(current_pos);

    return transpose;
}

SparseMatrix* add_sparse_and_transpose_binary(SparseMatrix *matrix) {
    // Transpose the matrix
    SparseMatrix *transpose = transpose_sparse_matrix(matrix);

    // Allocate memory for the result matrix
    SparseMatrix *result = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    result->rows = matrix->rows;
    result->cols = matrix->cols;
    result->nnz = 0; // Will calculate the number of non-zero elements dynamically

    // Temporary arrays to store the result in COO format before converting to CSR
    int max_nnz = matrix->nnz + transpose->nnz;
    double *temp_values = (double *)malloc(max_nnz * sizeof(double));
    int *temp_col_indices = (int *)malloc(max_nnz * sizeof(int));
    int *temp_row_ptr = (int *)calloc((matrix->rows + 1), sizeof(int));

    // Step 1: Iterate through each row and merge corresponding elements from both matrices
    int temp_nnz = 0;
    for (int row = 0; row < matrix->rows; row++) {
        int i = matrix->row_ptr[row];
        int j = transpose->row_ptr[row];

        while (i < matrix->row_ptr[row + 1] || j < transpose->row_ptr[row + 1]) {
            int col_i = (i < matrix->row_ptr[row + 1]) ? matrix->col_indices[i] : matrix->cols;
            int col_j = (j < transpose->row_ptr[row + 1]) ? transpose->col_indices[j] : matrix->cols;

            if (col_i == col_j) {
                // If either matrix has a 1, set the result to 1
                temp_values[temp_nnz] = 1.0;
                temp_col_indices[temp_nnz] = col_i;
                i++;
                j++;
            } else if (col_i < col_j) {
                // Take value from the original matrix
                temp_values[temp_nnz] = 1.0;
                temp_col_indices[temp_nnz] = col_i;
                i++;
            } else {
                // Take value from the transpose matrix
                temp_values[temp_nnz] = 1.0;
                temp_col_indices[temp_nnz] = col_j;
                j++;
            }

            temp_nnz++;
        }

        temp_row_ptr[row + 1] = temp_nnz;
    }

    // Step 2: Allocate memory for the result matrix arrays
    result->values = (double *)malloc(temp_nnz * sizeof(double));
    result->col_indices = (int *)malloc(temp_nnz * sizeof(int));
    result->row_ptr = (int *)malloc((matrix->rows + 1) * sizeof(int));
    result->nnz = temp_nnz;

    // Step 3: Copy the temporary arrays into the result matrix
    for (int i = 0; i < temp_nnz; i++) {
        result->values[i] = temp_values[i];
        result->col_indices[i] = temp_col_indices[i];
    }
    for (int i = 0; i <= matrix->rows; i++) {
        result->row_ptr[i] = temp_row_ptr[i];
    }

    // Free temporary memory
    free(temp_values);
    free(temp_col_indices);
    free(temp_row_ptr); 

    // Free the transpose matrix
    free_sparse_matrix(transpose);
    
    // Free the original matrix
    free_sparse_matrix(matrix);

    return result;
}

SparseMatrix* create_degree_matrix(SparseMatrix *adj_matrix) {
    // Allocate memory for the degree matrix
    SparseMatrix *degree_matrix = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    degree_matrix->rows = adj_matrix->rows;
    degree_matrix->cols = adj_matrix->cols;
    degree_matrix->nnz = adj_matrix->rows; // Only diagonal elements are non-zero

    // Allocate memory for the degree matrix arrays
    degree_matrix->values = (double *)malloc(degree_matrix->nnz * sizeof(double));
    degree_matrix->col_indices = (int *)malloc(degree_matrix->nnz * sizeof(int));
    degree_matrix->row_ptr = (int *)malloc((degree_matrix->rows + 1) * sizeof(int));

    // Step 1: Populate the degree matrix
    for (int row = 0; row < adj_matrix->rows; row++) {
        // Degree is the number of non-zero elements in the row
        int degree = adj_matrix->row_ptr[row + 1] - adj_matrix->row_ptr[row];

        // Set the diagonal element
        degree_matrix->values[row] = (double)degree;
        degree_matrix->col_indices[row] = row; // Diagonal element
        degree_matrix->row_ptr[row] = row;     // Start of the row
    }

    // Set the last element of row_ptr
    degree_matrix->row_ptr[degree_matrix->rows] = degree_matrix->nnz;

    return degree_matrix;
}

