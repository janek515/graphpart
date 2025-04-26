#ifndef PARTITIONER_H
#define PARTITIONER_H
#include "spectral_algorithm.h"
#include "graph.h"
#include "matrix_ops.h"

typedef struct {
    int num_vertices ; // Liczba wierzcholkow
    int num_parts ; // Liczba partycji
    int *partition ; // Tablica przypisania wierzcholkow do partycji
    int cut_edges ; // Liczba krawedzi przecietych
    float imbalance ; // Osiagniety wspolczynnik nierownowagl
    int *part_sizes ; // Liczba wierzcholkow w kazdej partycji
} PartitionResult ;
    
PartitionResult* create_partition_result(Graph *graph, int num_parts);
void free_partition_result(PartitionResult *result);
PartitionResult *spectral_partition(Graph* graph, int num_parts, float max_imbalance, int num_attempts);
void calculate_cut_edges(Graph *graph, PartitionResult *result);
void calculate_imbalance(PartitionResult *result);
void optimize_partition(Graph *graph, PartitionResult *result, float max_imbalance);
void print_partition_result(PartitionResult *result);
float get_minimum_achievable_imbalance(int num_vertices, int num_parts);

#endif
