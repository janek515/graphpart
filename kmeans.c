#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include "kmeans.h"

double squared_distance(double *point1, double *point2, int num_features) {
    double distance = 0.0;
    for (int i = 0; i < num_features; i++) {
        double diff = point1[i] - point2[i];
        distance += diff * diff;
    }
    return distance;
}

int* kmeans_clustering(double **spectral_points, int num_vertices, int num_eigenvectors, int num_parts) {
    int *labels = (int *)malloc(num_vertices * sizeof(int));
    double **centroids = (double **)malloc(num_parts * sizeof(double *));
    if (!centroids) {
        printf("Błąd: Nie udało się alokować pamięci dla centroidów.\n");
        free(labels);
        return NULL;
    }

    for (int i = 0; i < num_parts; i++) {
        centroids[i] = (double *)calloc(num_eigenvectors, sizeof(double));
        if (!centroids[i]) {
            printf("Błąd: Nie udało się alokować pamięci dla centroidu %d.\n", i);
            for (int j = 0; j < i; j++) free(centroids[j]);  
            free(centroids);
            free(labels);
            return NULL;
        }
    }

    for (int i = 0; i < num_vertices; i++) {
        labels[i] = -1;
    }
    
    for (int i = 0; i < num_parts; i++) {
        int random_index = rand() % num_vertices;
        for (int j = 0; j < num_eigenvectors; j++) {
            centroids[i][j] = spectral_points[random_index][j];
        }
    }

    int max_iterations = 100;
    for (int iter = 0; iter < max_iterations; iter++) {
        int converged = 1;

        for (int i = 0; i < num_vertices; i++) {
            double min_distance = DBL_MAX;
            int best_cluster = -1;
            for (int c = 0; c < num_parts; c++) {
                double distance = squared_distance(spectral_points[i], centroids[c], num_eigenvectors);
                if (distance < min_distance) {
                    min_distance = distance;
                    best_cluster = c;
                }
            }
            if (labels[i] != best_cluster) {
                labels[i] = best_cluster;
                converged = 0;
            }
        }

        int *cluster_sizes = (int *)calloc(num_parts, sizeof(int));
        for (int c = 0; c < num_parts; c++) {
            for (int j = 0; j < num_eigenvectors; j++) {
                centroids[c][j] = 0.0;
            }
        }
        for (int i = 0; i < num_vertices; i++) {
            int cluster = labels[i];
            cluster_sizes[cluster]++;
            for (int j = 0; j < num_eigenvectors; j++) {
                centroids[cluster][j] += spectral_points[i][j];
            }
        }
        for (int c = 0; c < num_parts; c++) {
            if (cluster_sizes[c] > 0) {
                for (int j = 0; j < num_eigenvectors; j++) {
                    centroids[c][j] /= cluster_sizes[c];
                }
            }
        }

        free(cluster_sizes);

        if (converged) {
            break;
        }
    }
    for (int i = 0; i < num_parts; i++) {
        free(centroids[i]);
    }
    free(centroids);

    return labels;
}

