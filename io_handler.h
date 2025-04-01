#ifndef IO_HANDLER_H
#define IO_HANDLER_H

typedef struct {
    int num_vertices ; // Liczba wierzcholkow
    int num_edges ; // Liczba krawedzi
    int max_row_nodes ; // Maksymalna liczba wezlow w wierszu
    int **edge_groups ; // Grupy wezlow polaczonych krawedziami
    int *group_ptr ; // Wskazniki na poczatki grup
    int num_groups ; // Liczba grup
    int *group_sizes ; // Liczba krawedzi w kazdej grupie, niekoniecznie potrzebne bo info jest w edge_groups
} Graph;

Graph* read_file(char *filename);
int get_last_value(char *buffer);
void free_memory(Graph *graph);

#endif
