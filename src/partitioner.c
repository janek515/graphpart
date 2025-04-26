#include "partitioner.h"
#include "log_utils.h"
#include "printfcolor.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PartitionResult *create_partition_result(Graph *graph, int num_parts) {
    if (!graph || num_parts <= 0) {
        error("Niepoprawne dane wejściowe dla create_partition_result.\n");
        return NULL;
    }

    PartitionResult *result = malloc(sizeof(PartitionResult));
    if (!result) {
        error("Nie udało się zaalokować pamięci dla struktury PartitionResult.\n");
        return NULL;
    }

    result->num_vertices = graph->num_vertices;
    result->num_parts = num_parts;

    result->partition = calloc(graph->num_vertices, sizeof(int));
    if (!result->partition) {
        error("Nie udało się zaalokować pamięci dla tablicy partition.\n");
        free(result);
        return NULL;
    }

    result->part_sizes = calloc(num_parts, sizeof(int));
    if (!result->part_sizes) {
        error("Nie udało się zaalokować pamięci dla tablicy part_sizes.\n");
        free(result->partition);
        free(result);
        return NULL;
    }

    result->cut_edges = 0;
    result->imbalance = 0.0f;

    return result;
}

void free_partition_result(PartitionResult *result) {
    if (result) {
        if (result->partition) {
            free(result->partition);
        }
        if (result->part_sizes) {
            free(result->part_sizes);
        }
        free(result);
    }
}

float get_minimum_achievable_imbalance(int num_vertices, int num_parts) {
    int ideal_size = num_vertices / num_parts;

    if (num_vertices % num_parts == 0) {
        return 1.0;
    }

    int remainder = num_vertices % num_parts;
    int max_part_size = ideal_size + (remainder > 0 ? 1 : 0);
    return (float)max_part_size / ideal_size;
}

PartitionResult *spectral_partition(Graph *graph, int num_parts, float max_imbalance,
                                    int num_attempts) {
    float min_achievable_imbalance =
        get_minimum_achievable_imbalance(graph->num_vertices, num_parts);
    if (min_achievable_imbalance > max_imbalance) {
        error("Maksymalny współczynnik nierównowagi %.2f jest niemożliwy do osiągnięcia.\n",
              max_imbalance);
        error("Najmniejszy możliwy współczynnik nierównowagi dla %d wierzchołków i %d partycji to "
              "%.5f\n",
              graph->num_vertices, num_parts, min_achievable_imbalance);
        return NULL;
    }
    SparseMatrix *matrix = create_adjacency_matrix(graph);
    SparseMatrix *temp = add_sparse_and_transpose_binary(matrix);
    if (!temp) {
        error("Failed to process adjacency matrix.\n");
        free_sparse_matrix(matrix);
    }

    free_sparse_matrix(matrix);
    matrix = temp;

    verbose("Tworzenie macierzy Laplace'a ");
    fflush(stdout);
    SparseMatrix *laplacian = build_laplacian_matrix(matrix);
    printfc_fg(GREY, "skończone.\n");

    int num_eigenvectors = num_parts - 1;
    verbose("Przetwarzanie wektorów własnych ");
    fflush(stdout);
    DenseVector **eigenvectors = compute_eigenvectors(laplacian, num_eigenvectors);
    printfc_fg(GREY, "skończone.\n");

    double **spectral_points = malloc(graph->num_vertices * sizeof(double *));
    for (int i = 0; i < graph->num_vertices; i++) {
        spectral_points[i] = malloc(num_eigenvectors * sizeof(double));
        for (int j = 0; j < num_eigenvectors; j++) {
            spectral_points[i][j] = eigenvectors[j]->values[i];
        }
    }
    PartitionResult *best_result = NULL;
    int min_cut_edges = INT_MAX;

    for (int attempt = 0; attempt < num_attempts; attempt++) {
        int *clusters =
            kmeans_clustering(spectral_points, graph->num_vertices, num_eigenvectors, num_parts);

        PartitionResult *current_result = create_partition_result(graph, num_parts);
        for (int i = 0; i < graph->num_vertices; i++) {
            current_result->partition[i] = clusters[i];
        }

        optimize_partition(graph, current_result, max_imbalance);
        calculate_cut_edges(graph, current_result);
        calculate_imbalance(current_result);

        if (current_result->cut_edges < min_cut_edges &&
            current_result->imbalance <= max_imbalance) {
            if (best_result) {
                free_partition_result(best_result);
            }
            best_result = current_result;
            min_cut_edges = current_result->cut_edges;
            verbose("Znaleziono lepsze rozwiązanie: przecięte krawędzie = %d, nierównowaga = %.2f\n",
                 min_cut_edges, current_result->imbalance);
        } else {
            free_partition_result(current_result);
        }
        free(clusters);
    }

    free_sparse_matrix(matrix);
    free_sparse_matrix(laplacian);
    free_eigenvectors(eigenvectors, num_eigenvectors);
    for (int i = 0; i < graph->num_vertices; i++) {
        free(spectral_points[i]);
    }
    free(spectral_points);

    return best_result;
}

void optimize_partition(Graph *graph, PartitionResult *result, float max_imbalance) {
    if (!graph || !result) {
        error("Niepoprawne dane wejściowe do optimize_partition.\n");
        return;
    }

    int num_vertices = graph->num_vertices;
    int num_parts = result->num_parts;
    int *partition = result->partition;
    int *part_sizes = result->part_sizes;
    int ideal_size = num_vertices / num_parts;
    int max_allowed_size = (int)(ideal_size * max_imbalance);

    memset(part_sizes, 0, num_parts * sizeof(int));

    for (int i = 0; i < num_vertices; i++) {
        if (partition[i] < 0 || partition[i] >= num_parts) {
            error("Niepoprawny indeks partycji dla wierzchołka %d.\n", i);
            return;
        }
        part_sizes[partition[i]]++;
    }

    for (int p = 0; p < num_parts; p++) {
        while (part_sizes[p] > max_allowed_size) {
            // znajdz wierzcholek do przesuniecia
            for (int v = 0; v < num_vertices; v++) {
                if (partition[v] == p) {
                    for (int other_p = 0; other_p < num_parts; other_p++) {
                        if (other_p != p && part_sizes[other_p] < max_allowed_size) {
                            partition[v] = other_p;
                            part_sizes[p]--;
                            part_sizes[other_p]++;
                            break;
                        }
                    }
                    if (part_sizes[p] <= max_allowed_size) {
                        break;
                    }
                }
            }
        }
    }

    int improved = 1;
    while (improved) {
        improved = 0;

        for (int v = 0; v < num_vertices; v++) {
            int current_part = partition[v];
            int best_part = current_part;
            int min_cut_increase = 0;

            for (int p = 0; p < num_parts; p++) {
                if (p == current_part) {
                    continue;
                }

                if (part_sizes[p] + 1 > max_allowed_size) {
                    continue;
                }

                int cut_increase = 0;
                if (graph->edge_groups[v] == NULL) {
                    continue;
                }

                for (int i = 0; i < graph->group_sizes[v]; i++) {
                    int neighbor = graph->edge_groups[v][i];
                    if (neighbor < 0 || neighbor >= num_vertices) {
                        error("Niepoprawny sąsiad dla wierzchołka %d.\n", v);
                        return;
                    }

                    if (partition[neighbor] == p) {
                        cut_increase--;
                    } else if (partition[neighbor] == current_part) {
                        cut_increase++;
                    }
                }

                if (cut_increase < min_cut_increase) {
                    min_cut_increase = cut_increase;
                    best_part = p;
                }
            }

            // jesli lepsza czesc to zmieniamy
            if (best_part != current_part) {
                partition[v] = best_part;
                part_sizes[current_part]--;
                part_sizes[best_part]++;
                improved = 1;
            }
        }
    }
}

void calculate_cut_edges(Graph *graph, PartitionResult *result) {
    if (!graph || !result) {
        error("Niepoprawne dane wejściowe do calculate_cut_edges.\n");
        return;
    }

    int cut_edges = 0;
    for (int v = 0; v < graph->num_vertices; v++) {
        if (graph->edge_groups[v] == NULL) {
            continue;
        }

        for (int i = 0; i < graph->group_sizes[v]; i++) {
            int neighbor = graph->edge_groups[v][i];
            if (result->partition[v] != result->partition[neighbor]) {
                cut_edges++;
            }
        }
    }
    result->cut_edges = cut_edges / 2;
}

void calculate_imbalance(PartitionResult *result) {
    if (!result) {
        error("Niepoprawne dane wejściowe do calculate_imbalance.\n");
        return;
    }

    int total_vertices = result->num_vertices;
    int num_parts = result->num_parts;
    int ideal_size = total_vertices / num_parts;
    int max_size = 0;

    for (int p = 0; p < num_parts; p++) {
        if (result->part_sizes[p] > max_size) {
            max_size = result->part_sizes[p];
        }
    }
    result->imbalance = (float)max_size / ideal_size;
}

void print_partition_result(PartitionResult *result) {
    if (!result) {
        error("Niepoprawny PartitionResult.\n");
        return;
    }
    info("%d %d\n", result->num_vertices, result->num_parts);

    for (int i = 0; i < result->num_vertices; i++) {
        info("%d\n", result->partition[i]);
    }

    info("# Statystyki podziału:\n");
    info("# Liczba krawędzi przeciętych: %d\n", result->cut_edges);
    info("# Współczynnik nierównowagi: %.2f\n", result->imbalance);
}
