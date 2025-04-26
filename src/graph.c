#include "graph.h"
#include "log_utils.h"
#include <stdio.h>
#include <stdlib.h>

SparseMatrix *create_adjacency_matrix(Graph *graph) {
    SparseMatrix *matrix = malloc(sizeof(SparseMatrix));
    if (!matrix) {
        error("Nie udało się zaalokować pamięci dla SparseMatrix.\n");
        return NULL;
    }

    matrix->rows = graph->num_vertices;
    matrix->cols = graph->num_vertices;
    matrix->nnz = graph->num_edges;

    matrix->values = malloc(matrix->nnz * sizeof(double));
    matrix->col_indices = malloc(matrix->nnz * sizeof(int));
    matrix->row_ptr = malloc((matrix->rows + 1) * sizeof(int));

    if (!matrix->values || !matrix->col_indices || !matrix->row_ptr) {
        error("Nie udało się zaalokować pamięci dla elementów macierzy.\n");
        free(matrix->values);
        free(matrix->col_indices);
        free(matrix->row_ptr);
        free(matrix);
        return NULL;
    }

    int edge_index = 0;
    matrix->row_ptr[0] = 0;

    for (int i = 0; i < graph->num_vertices; i++) {
        // printf("debug: group_sizes[%d] = %d\n", i, graph->group_sizes[i]);

        for (int j = 0; j < graph->group_sizes[i]; j++) {
            int col = graph->edge_groups[i][j];

            if (col < 0 || col >= graph->num_vertices) {
                error("Nieprawidłowy rozmiar kolumny %d dla wierzchołka "
                      "%d\n",
                      col, i);
                free_sparse_matrix(matrix);
                return NULL;
            }

            matrix->col_indices[edge_index] = col;
            matrix->values[edge_index] = 1.0;
            edge_index++;
        }
        matrix->row_ptr[i + 1] = edge_index;
    }
    return matrix;
}

void print_sparse_matrix(SparseMatrix *matrix) {
    info("Wartości niezerowych elementów:\n");
    info("");
    for (int i = 0; i < matrix->nnz; i++) {
        printf("%f ", matrix->values[i]);
    }
    printf("\n\n");

    info("Indeksy kolumn:\n");
    info("");
    for (int i = 0; i < matrix->nnz; i++) {
        printf("%d ", matrix->col_indices[i]);
    }
    printf("\n\n");

    info("Wskaźniki wierszy:\n");
    info("");
    for (int i = 0; i <= matrix->rows; i++) {
        printf("%d ", matrix->row_ptr[i]);
    }
    printf("\n");
}

void print_dense_matrix(SparseMatrix *matrix) {
    if (!matrix) {
        error("Macierz jest NULL.\n");
        return;
    }

    info("Macierz: \n");

    for (int i = 0; i < matrix->rows; i++) {
        int start = matrix->row_ptr[i];
        int end = matrix->row_ptr[i + 1];

        double *row = calloc(matrix->cols, sizeof(double));
        if (!row) {
            error("Failed to allocate memory for row buffer\n");
            return;
        }

        for (int j = start; j < end; j++) {
            row[matrix->col_indices[j]] = matrix->values[j];
        }

        for (int j = 0; j < matrix->cols; j++) {
            printf("%2.0f ", row[j]);
        }
        printf("\n");
        free(row);
    }
}

void free_sparse_matrix(SparseMatrix *matrix) {
    if (matrix) {
        free(matrix->values);
        free(matrix->col_indices);
        free(matrix->row_ptr);
        free(matrix);
    }
}

void print_graph_details(Graph *graph) {
    if (!graph) {
        error("Graf jest NULL.\n");
        return;
    }

    info("Szczegóły grafu:\n");
    info("Liczba wierzchołków: %d\n", graph->num_vertices);
    info("Liczba krawędzi: %d\n", graph->num_edges);
    info("Liczba grup: %d\n", graph->num_groups);
}
