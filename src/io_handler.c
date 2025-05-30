#include "io_handler.h"
#include "log_utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1000000

int compare_ints(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

Graph *read_graph_from_file(FILE *file) {
    if (!file) {
        error("Nieprawidłowy wskaźnik pliku\n");
        return NULL;
    }

    Graph *graph = malloc(sizeof(Graph));
    if (!graph) {
        error("Nie udało się zaalokować pamięci dla grafu\n");
        return NULL;
    }

    char buffer[MAX];
    char line2_copy[MAX];
    char line3_copy[MAX];
    char line4_copy[MAX];
    char line5_copy[MAX];

    // max wierzcholkow w wierszu
    if (fscanf(file, "%d\n", &graph->max_row_nodes) != 1) {
        free(graph);
        return NULL; // koniec linii lub blad
    }

    // kolumna w ktorej znajduje sie dany wierzcholek, na razie kopiuje zeby
    // uzyc jak policze liczbe wierzcholkow
    if (!fgets(buffer, sizeof(buffer), file)) {
        free(graph);
        return NULL;
    }
    strcpy(line2_copy, buffer);
    int num_vertices = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';') {
            num_vertices++;
        }
    }
    num_vertices++;
    graph->num_vertices = num_vertices;

    // wiersz w ktorym znajduje sie dany wierzcholek
    if (!fgets(buffer, sizeof(buffer), file)) {
        free(graph);
        return NULL;
    }
    strcpy(line3_copy, buffer);

    // przetworzenie wierszy majac liczbe wierzcholkow
    graph->row = malloc(graph->num_vertices * sizeof(int));
    if (!graph->row) {
        error("Nie udało się zaalokować pamięci dla row.\n");
        free(graph);
        return NULL;
    }
    char *row_token = strtok(line3_copy, ";");
    int prev_position = atoi(row_token);
    int curr_position;
    row_token = strtok(NULL, ";");
    int current_row = 0;

    for (int v = 0; v < graph->num_vertices;) {
        if (!row_token) {
            break;
        }

        curr_position = atoi(row_token);
        row_token = strtok(NULL, ";");

        if (curr_position == prev_position) {
            current_row++;
            prev_position = curr_position;
            continue;
        }

        for (int i = prev_position; i < curr_position && v < graph->num_vertices; i++, v++) {
            graph->row[i] = current_row;
        }

        prev_position = curr_position;
        current_row++;
    }

    // majac liczbe wierzcholkow mozna przetworzyc kolumny
    graph->col = malloc(graph->num_vertices * sizeof(int));
    if (!graph->col) {
        error("Nie udało się zaalokować pamięci dla col.\n");
        free(graph->row);
        free(graph);
        return NULL;
    }
    char *tok = strtok(line2_copy, ";");
    int vertex_idx = 0;
    while (tok != NULL && vertex_idx < graph->num_vertices) {
        graph->col[vertex_idx++] = atoi(tok);
        tok = strtok(NULL, ";");
    }

    // linia 4
    if (!fgets(buffer, sizeof(buffer), file)) {
        free(graph->col);
        free(graph->row);
        free(graph);
        return NULL;
    }

    int count = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';') {
            count++;
        }
    }
    count++;
    strcpy(line4_copy, buffer);

    // linia 5
    if (!fgets(buffer, sizeof(buffer), file)) {
        free(graph->col);
        free(graph->row);
        free(graph);
        return NULL;
    }
    strcpy(line5_copy, buffer);

    int group_count = 1;
    for (char *p = buffer; *p; p++) {
        if (*p == ';') {
            group_count++;
        }
    }
    graph->num_groups = group_count;

    graph->group_ptr = malloc((graph->num_groups + 1) * sizeof(int));
    if (!graph->group_ptr) {
        error("Nie udało się alokować pamięci dla group_ptr.\n");
        free(graph->col);
        free(graph->row);
        free(graph);
        return NULL;
    }

    char *token = strtok(line5_copy, ";");
    int group_idx = 0;
    while (token != NULL && group_idx < graph->num_groups) {
        graph->group_ptr[group_idx++] = atoi(token);
        token = strtok(NULL, ";");
    }

    graph->edge_groups = malloc(graph->num_vertices * sizeof(int *));
    if (!graph->edge_groups) {
        error("Nie udało się alokować pamięci dla edge_groups.\n");
        free(graph->group_ptr);
        free(graph->col);
        free(graph->row);
        free(graph);
        return NULL;
    }

    graph->group_sizes = malloc(graph->num_vertices * sizeof(int));
    if (!graph->group_sizes) {
        error("Nie udało się alokować pamięci dla group_sizes.\n");
        free(graph->edge_groups);
        free(graph->group_ptr);
        free(graph->col);
        free(graph->row);
        free(graph);
        return NULL;
    }

    token = strtok(line4_copy, ";");
    graph->num_edges = 0;
    int k = 0;

    for (int i = 0; i < graph->num_vertices; i++) {
        if (k < graph->num_groups && atoi(token) == i) { // sprawdza czy pierwszy token jest rowny
                                                         // indeksowi wierzcholka
            int start_idx = (k < graph->num_groups) ? graph->group_ptr[k] : 0;
            int end_idx = (k < graph->num_groups - 1) ? graph->group_ptr[k + 1] : count;

            graph->group_sizes[i] = (end_idx > start_idx) ? (end_idx - start_idx - 1) : 0;
            graph->num_edges += graph->group_sizes[i];

            if (graph->group_sizes[i] > 0) {
                graph->edge_groups[i] = malloc(graph->group_sizes[i] * sizeof(int));
                if (!graph->edge_groups[i]) {
                    error("Nie udało się alokować pamięci dla "
                          "edge_groups[%d]\n",
                          i);
                    for (int j = 0; j < i; j++) {
                        if (graph->edge_groups[j]) {
                            free(graph->edge_groups[j]);
                        }
                    }
                    free(graph->edge_groups);
                    free(graph->group_sizes);
                    free(graph->group_ptr);
                    free(graph->col);
                    free(graph->row);
                    free(graph);
                    return NULL;
                }
            } else {
                graph->edge_groups[i] = NULL;
            }

            token = strtok(NULL,
                           ";"); // pomin pierwszy elemnent (bo to indeks wierzcholka)
            for (int j = 0; j < graph->group_sizes[i]; j++) {
                if (token == NULL) {
                    error("Niespodziewany koniec bufora line4_copy podczas "
                          "prztwarzania edge_groups.\n");
                    for (int k = 0; k <= i; k++) {
                        if (graph->edge_groups[k]) {
                            free(graph->edge_groups[k]);
                        }
                    }
                    free(graph->edge_groups);
                    free(graph->group_sizes);
                    free(graph->group_ptr);
                    free(graph->col);
                    free(graph->row);
                    free(graph);
                    return NULL;
                }
                graph->edge_groups[i][j] = atoi(token);
                token = strtok(NULL, ";");
            }
            // sortuje kazda grupe
            qsort(graph->edge_groups[i], graph->group_sizes[i], sizeof(int), compare_ints);
            k++;
        } else {
            graph->group_sizes[i] = 0;
            graph->edge_groups[i] = NULL;
        }
    }
    return graph;
}

Graph **read_multiple_graphs(char *filename, int *num_graphs) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        error("Nie można otworzyć pliku '%s'\n", filename);
        return NULL;
    }

    Graph **graphs = NULL;
    int capacity = 5; // poczatkowa pojemnosc
    *num_graphs = 0;

    graphs = malloc(capacity * sizeof(Graph *));
    if (!graphs) {
        error("Nie udało się zaalokować pamięci dla tablicy grafów\n");
        fclose(file);
        return NULL;
    }
    Graph *current_graph;
    while (!feof(file)) {
        int c = fgetc(file);
        if (c == EOF) {
            break;
        }
        if (c == '#') {
            while ((c = fgetc(file)) != EOF && c != '\n')
                ;
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            continue;
        }
        ungetc(c, file);
        current_graph = read_graph_from_file(file);
        if (current_graph != NULL) {
            if (*num_graphs >= capacity) {
                capacity *= 2;
                Graph **temp = (Graph **)realloc(graphs, capacity * sizeof(Graph *));
                if (!temp) {
                    error("Nie udało się zaalokować pamięci dla tablicy "
                          "grafów\n");
                    free_multiple_graphs(graphs, *num_graphs);
                    fclose(file);
                    return NULL;
                }
                graphs = temp;
            }
            graphs[*num_graphs] = current_graph;
            (*num_graphs)++;
        }
    }
    fclose(file);
    if (*num_graphs == 0) {
        free(graphs);
        return NULL;
    }
    if (*num_graphs < capacity) {
        Graph **temp = (Graph **)realloc(graphs, (*num_graphs) * sizeof(Graph *));
        if (temp) {
            graphs = temp;
        }
    }

    return graphs;
}

void free_memory(Graph *graph) {
    if (graph) {
        if (graph->edge_groups) {
            for (int i = 0; i < graph->num_vertices; i++) {
                if (graph->edge_groups[i]) { // sprawdza czy nie null
                    free(graph->edge_groups[i]);
                }
            }
            free(graph->edge_groups);
        }

        free(graph->group_sizes);
        free(graph->group_ptr);
        free(graph->row);
        free(graph->col);
        free(graph);
    }
}

void free_multiple_graphs(Graph **graphs, int num_graphs) {
    if (graphs == NULL) {
        return;
    }
    for (int i = 0; i < num_graphs; i++) {
        if (graphs[i] != NULL) {
            free_memory(graphs[i]);
        }
    }
    free(graphs);
}

void save_in_text_file(PartitionResult *result, char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        error("Nie można otworyć pliku '%s'\n", filename);
        return;
    }
    fprintf(file, "%d %d\n", result->num_vertices, result->num_parts);

    for (int i = 0; i < result->num_vertices; i++) {
        fprintf(file, "%d\n", result->partition[i]);
    }

    fprintf(file, "# Statystyki partycjonowania:\n");
    fprintf(file, "# Liczba krawędzi przeciętych: %d\n", result->cut_edges);
    fprintf(file, "# Współczynnik nierównowagi: %.5f\n", result->imbalance);

    fclose(file);
    info("Udało się zapisać wynik partycjonowania w pliku tekstowym '%s'.\n", filename);
}

void save_in_binary_file(PartitionResult *result, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        error("Nie można otworzyć pliku '%s'.\n", filename);
        return;
    }

    int32_t num_vertices = (int32_t)result->num_vertices;
    int32_t num_parts = (int32_t)result->num_parts;
    fwrite(&num_vertices, sizeof(int32_t), 1, file);
    fwrite(&num_parts, sizeof(int32_t), 1, file);

    if (num_parts <= 256) {
        for (int i = 0; i < result->num_vertices; i++) {
            uint8_t part = (uint8_t)result->partition[i];
            fwrite(&part, sizeof(uint8_t), 1, file);
        }
    } else {
        for (int i = 0; i < result->num_vertices; i++) {
            uint16_t part = (uint16_t)result->partition[i];
            fwrite(&part, sizeof(uint16_t), 1, file);
        }
    }
    int32_t cut_edges = (int32_t)result->cut_edges;
    float imbalance = result->imbalance;

    fwrite(&cut_edges, sizeof(int32_t), 1, file);
    fwrite(&imbalance, sizeof(float), 1, file);

    fclose(file);
    info("Udało się zapisać wynik partycjonowania w pliku binarnym '%s'.\n", filename);
}
