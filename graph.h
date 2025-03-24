#ifndef GRAPH_H
#define GRAPH_H

typedef struct {
    int max_nodes_per_row; // Maksymalna liczba węzłów w wierszu
    int num_nodes;         // Liczba węzłów w grafie
    int num_edges;         // Liczba krawędzi
    int *node_indices;     // Indeksy węzłów w wierszach
    int *row_pointers;     // Wskaźniki na początki wierszy
    int *edge_groups;      // Lista węzłów połączonych krawędziami
    int *group_pointers;   // Wskaźniki na początki grup krawędziowych
} Graph;

Graph* read_file(char *filename);

#endif
