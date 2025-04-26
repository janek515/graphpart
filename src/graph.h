#ifndef GRAPH_H
#define GRAPH_H

typedef struct {
    int num_vertices;  // Liczba wierzcholkow
    int num_edges;     // Liczba krawedzi
    int *row;          // Wiersz, w ktorym znajduje sie dany wierzcholek
    int *col;          // Kolumna, w ktorej znajduje sie dany wierzcholek
    int max_row_nodes; // Maksymalna liczba wezlow w wierszu
    int **edge_groups; // Grupy wezlow polaczonych krawedziami
    int *group_ptr;    // Wskazniki na poczatki grup
    int num_groups;    // Liczba grup
    int *group_sizes;  // Liczba krawedzi w kazdej grupie, niekoniecznie
                       // potrzebne bo info jest w edge_groups
} Graph;

typedef struct {
    int rows;         // Liczba wierszy
    int cols;         // Liczba kolumn
    int nnz;          // Liczba niezerowych elementow
    double *values;   // Wartosci niezerowych elementow
    int *col_indices; // Indeksy kolumn
    int *row_ptr;     // Wskazniki wierszy
} SparseMatrix;

SparseMatrix *create_adjacency_matrix(Graph *graph);
void print_sparse_matrix(SparseMatrix *matrix);
void free_sparse_matrix(SparseMatrix *matrix);
void print_dense_matrix(SparseMatrix *matrix);
void print_graph_details(Graph *graph);

#endif
