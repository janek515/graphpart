#ifndef IO_HANDLER_H
#define IO_HANDLER_H
#include "partitioner.h"
#include "graph.h"

Graph* read_graph_from_file(FILE *file);
Graph** read_multiple_graphs(char *filename, int *num_graphs);
void free_memory(Graph *graph);
void free_multiple_graphs(Graph **graphs, int num_graphs);
void save_in_text_file(PartitionResult *result, char *filename);
void save_in_binary_file(PartitionResult *result, char *filename);
void save_in_csrrg_format(PartitionResult *result, Graph *graph, char *filename);

#endif
