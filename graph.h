#ifndef GRAPH_H
#define GRAPH_H

typedef struct {
    int num_vertices ; // Liczba wierzcholkow
    int num_edges ; // Liczba krawedzi
    int *row_ptr; // Wskazniki na poczatki wierszy (n+1 elementow)
    int *col_indices ; // Indeksy kolumn (2*m elementow)
    int max_row_nodes ; // Maksymalna liczba wezlow w wierszu
    int **edge_groups ; // Grupy wezlow polaczonych krawedziami
    int *group_ptr ; // Wskazniki na poczatki grup
    int num_groups ; // Liczba grup
} Graph;

Graph* read_file(char *filename);
int get_last_value(char *buffer);
void free_memory(Graph *g);

#endif
