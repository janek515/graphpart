#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

Graph* read_file(char *filename) {
   FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Błąd: Nie można otworzyć pliku '%s'\n", filename);
        exit(1);
    }

    Graph *g = (Graph*)malloc(sizeof(Graph));
    if (!g) {
        printf("Błąd: Nie udało się zaalokować pamięci dla grafu\n");
        fclose(file);
        exit(0);
    }

    // max_nodes_per_row
    fscanf(file, "%d\n", &g->max_nodes_per_row);

    char buffer[4096];  // bufor do odczytu linii
    char *token;

    // node_indices
    fgets(buffer, sizeof(buffer), file);
    int actual_nodes = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';')
            actual_nodes++;
    }
    actual_nodes++;

    g->num_nodes = actual_nodes;
    g->node_indices = (int*)malloc(g->num_nodes * sizeof(int));

    token = strtok(buffer, ";");
    int count = 0;
    while (token != NULL) {
        g->node_indices[count++] = atoi(token);
        token = strtok(NULL, ";");
    }

    // row_pointers
    fgets(buffer, sizeof(buffer), file);
    int row_pointers_count = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';') row_pointers_count++;
    }
    row_pointers_count++;

    g->row_pointers = (int*)malloc(row_pointers_count * sizeof(int));

    token = strtok(buffer, ";");
    count = 0;
    while (token != NULL) {
        g->row_pointers[count++] = atoi(token);
        token = strtok(NULL, ";");
    }

    // Wczytaj edge_groups
    fgets(buffer, sizeof(buffer), file);
    int edge_count = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';')
            edge_count++;
    }
    edge_count++;

    g->num_edges = edge_count;
    g->edge_groups = (int*)malloc(g->num_edges * sizeof(int));

    token = strtok(buffer, ";");
    count = 0;
    while (token != NULL) {
        g->edge_groups[count++] = atoi(token);
        token = strtok(NULL, ";");
    }

    //group_pointers
    fgets(buffer, sizeof(buffer), file);
    int group_pointers_count = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';') group_pointers_count++;
    }
    group_pointers_count++;

    g->group_pointers = (int*)malloc(group_pointers_count * sizeof(int));

    token = strtok(buffer, ";");
    count = 0;
    while (token != NULL) {
        g->group_pointers[count++] = atoi(token);
        token = strtok(NULL, ";");
    }

    fclose(file);
    return g;
}
