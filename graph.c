#include <stdio.h>
#include <stdlib.h>
#include "graph.h"

SparseMatrix* create_adjacency_matrix(Graph *graph) 
{
    SparseMatrix *matrix = (SparseMatrix *)malloc(sizeof(SparseMatrix));
    if (!matrix) {
        printf("Error: Failed to allocate memory for SparseMatrix.\n");
        return NULL;
    }

    matrix->rows = graph->num_vertices;
    matrix->cols = graph->num_vertices;
    matrix->nnz = graph->num_edges;

    matrix->values = (double *)malloc(matrix->nnz * sizeof(double));
    matrix->col_indices = (int *)malloc(matrix->nnz * sizeof(int));
    matrix->row_ptr = (int *)malloc((matrix->rows + 1) * sizeof(int));

    if (!matrix->values || !matrix->col_indices || !matrix->row_ptr) {
        printf("Error: Failed to allocate memory for matrix components.\n");
        free(matrix->values);
        free(matrix->col_indices);
        free(matrix->row_ptr);
        free(matrix);
        return NULL;
    }

        // Fill CSR arrays
        int edge_index = 0;
        matrix->row_ptr[0] = 0; // First row always starts at index 0
    
        for (int i = 0; i < graph->num_vertices; i++) {
            //printf("Debug: Processing vertex %d\n", i);
            //printf("Debug: group_sizes[%d] = %d\n", i, graph->group_sizes[i]);
    
            for (int j = 0; j < graph->group_sizes[i]; j++) {
                int col = graph->edge_groups[i][j];
    
                // Validate column index
                if (col < 0 || col >= graph->num_vertices) {
                    printf("Error: Invalid column index %d for vertex %d\n", col, i);
                    free_sparse_matrix(matrix);
                    return NULL;
                }
    
                // Populate CSR arrays
                matrix->col_indices[edge_index] = col;
                matrix->values[edge_index] = 1.0; // Assuming unweighted graph
                //printf("Debug: Adding edge (%d -> %d) at index %d\n", i, col, edge_index);
                edge_index++;
            }
            matrix->row_ptr[i + 1] = edge_index; // Mark end of row
        }
        return matrix;
    }

void print_sparse_matrix(SparseMatrix *matrix) 
{
    printf("Wartości niezerowych elementów:\n");
    for (int i = 0; i < matrix->nnz; i++) {
        printf("%f ", matrix->values[i]);
    }
    printf("\n\nIndeksy kolumn:\n");
    for (int i = 0; i < matrix->nnz; i++) {
        printf("%d ", matrix->col_indices[i]);
    }
    printf("\n\nWskazniki wierszy:\n");
    for (int i = 0; i <= matrix->rows; i++) {
        printf("%d ", matrix->row_ptr[i]);
    }
    printf("\n");
}

void print_dense_adjacency_matrix(SparseMatrix *matrix) 
{
    printf("Adjacency Matrix (Dense Format):\n");

    for (int i = 0; i < matrix->rows; i++) {
        int start = matrix->row_ptr[i];
        int end = matrix->row_ptr[i + 1];

        // Create a row initialized to 0
        int *row = (int*)calloc(matrix->cols, sizeof(int));
        if (!row) {
            printf("Error: Failed to allocate memory for row buffer\n");
            return;
        }

        // Populate the row with non-zero values
        for (int j = start; j < end; j++) {
            row[matrix->col_indices[j]] = 1; // Adjacency matrix values are 1
        }

        // Print the row
        for (int j = 0; j < matrix->cols; j++) {
            printf("%d ", row[j]);
        }
        printf("\n");

        free(row); // Free the row buffer
    }
}

void print_dense_matrix(SparseMatrix *matrix) 
{
    if (!matrix) {
        printf("Error: Matrix is NULL\n");
        return;
    }

    printf("Matrix (Dense Format):\n");

    for (int i = 0; i < matrix->rows; i++) {
        int start = matrix->row_ptr[i];
        int end = matrix->row_ptr[i + 1];

        // Create a row initialized to 0
        int *row = (int *)calloc(matrix->cols, sizeof(int));
        if (!row) {
            printf("Error: Failed to allocate memory for row buffer\n");
            return;
        }

        // Populate the row with non-zero values
        for (int j = start; j < end; j++) {
            row[matrix->col_indices[j]] = (int)matrix->values[j];
        }

        // Print the row
        for (int j = 0; j < matrix->cols; j++) {
            printf("%d ", row[j]);
        }
        printf("\n");

        free(row); // Free the row buffer
    }
}

void free_sparse_matrix(SparseMatrix *matrix) 
{
    if (matrix) {
        free(matrix->values);
        free(matrix->col_indices);
        free(matrix->row_ptr);
        free(matrix);
    }
}