#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "io_handler.h"
#define MAX 1000000

int compare_ints(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

Graph* read_graph_from_file(FILE *file) {
    if (!file) {
        printf("Błąd: Nieprawidłowy wskaźnik pliku\n");
        return NULL;
    }

    Graph *graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) {
        printf("Błąd: Nie udało się zaalokować pamięci dla grafu\n");
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
        return NULL; // End of file or error
    }

    // kolumna w ktorej znajduje sie dany wierzcholek, na razie kopiuje zeby uzyc jak policze liczbe wierzcholkow
    if (!fgets(buffer, sizeof(buffer), file)) {
        free(graph);
        return NULL;
    }
    strcpy(line2_copy, buffer);
    int num_vertices = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';') num_vertices++;
    }
    num_vertices++;
    graph->num_vertices = num_vertices;

    // wiersz w ktorym znajduje sie dany wierzcholek
    if (!fgets(buffer, sizeof(buffer), file)) {
        free(graph);
        return NULL;
    }
    strcpy(line3_copy, buffer);

    //przetworzenie wierszy majac liczbe wierzcholkow
    graph->row = (int*)malloc(graph->num_vertices * sizeof(int));
    if (!graph->row) {
        printf("Błąd: Nie udało się zaalokować pamięci dla row.\n");
        free(graph);
        return NULL;
    }
    char *row_token = strtok(line3_copy, ";");
    int prev_position = atoi(row_token);
    int curr_position;
    row_token = strtok(NULL, ";");
    int current_row = 0;

    for (int v = 0; v < graph->num_vertices; ) {
        if (!row_token) break;
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

    //majac liczbe wierzcholkow mozna przetworzyc kolumny
    graph->col = (int*)malloc(graph->num_vertices * sizeof(int));
    if (!graph->col) {
        printf("Błąd: Nie udało się zaalokować pamięci dla col.\n");
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

    //linia 4
    if (!fgets(buffer, sizeof(buffer), file)) {
        free(graph->col);
        free(graph->row);
        free(graph);
        return NULL;
    }

    int count = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';') count++;
    }
    count++;
    strcpy(line4_copy, buffer);

    //linia 5
    if (!fgets(buffer, sizeof(buffer), file)) {
        free(graph->col);
        free(graph->row);
        free(graph);
        return NULL;
    }
    strcpy(line5_copy, buffer); 

    int group_count = 1;
    for(char *p = buffer; *p; p++) {
        if(*p == ';') group_count++;
    }
    graph->num_groups = group_count;

    graph->group_ptr = (int*)malloc((graph->num_groups+1) * sizeof(int));
    if (!graph->group_ptr) {
        printf("Błąd: Nie udało się alokować pamięci dla group_ptr.\n");
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

    graph->edge_groups = (int**)malloc(graph->num_vertices * sizeof(int*));
    if (!graph->edge_groups) {
        printf("Błąd: Nie udało się alokować pamięci dla edge_groups.\n");
        free(graph->group_ptr);
        free(graph->col);
        free(graph->row);
        free(graph);
        return NULL;
    }

    graph->group_sizes = (int*)malloc(graph->num_vertices * sizeof(int));
    if (!graph->group_sizes) {
        printf("Błąd: Nie udało się alokować pamięci dla group_sizes.\n");
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
        if (k < graph->num_groups && atoi(token) == i) { // sprawdza czy pierwszy token jest rowny indeksowi wierzcholka
            int start_idx = (k < graph->num_groups) ? graph->group_ptr[k] : 0;
            int end_idx = (k < graph->num_groups - 1) ? graph->group_ptr[k + 1] : count;

            graph->group_sizes[i] = (end_idx > start_idx) ? (end_idx - start_idx - 1) : 0;
            graph->num_edges += graph->group_sizes[i];
            
            if (graph->group_sizes[i] > 0) {
                graph->edge_groups[i] = (int*)malloc(graph->group_sizes[i] * sizeof(int));
                if (!graph->edge_groups[i]) {
                    printf("Błąd: Nie udało się alokować pamięci dla edge_groups[%d]\n", i);
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

            token = strtok(NULL, ";"); // pomin pierwszy elemnent (bo to indeks wierzcholka)
            for (int j = 0; j < graph->group_sizes[i]; j++) {
                if (token == NULL) {
                    printf("Błąd: Niespodziewany koniec bufora line4_copy podczas prztwarzania edge_groups.\n");
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
            //sortuje kazda grupe
            qsort(graph->edge_groups[i], graph->group_sizes[i], sizeof(int), compare_ints);
            k++; 
        } else {
            graph->group_sizes[i] = 0;
            graph->edge_groups[i] = NULL;
        }
    }
    return graph;
}

Graph** read_multiple_graphs(char *filename, int *num_graphs) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Błąd: Nie można otworzyć pliku '%s'\n", filename);
        return NULL;
    }

    Graph **graphs = NULL;
    int capacity = 5; // poczatkowa pojemnosc
    *num_graphs = 0;

    graphs = (Graph**)malloc(capacity * sizeof(Graph*));
    if (!graphs) {
        printf("Błąd: Nie udało się zaalokować pamięci dla tablicy grafów\n");
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
            while ((c = fgetc(file)) != EOF && c != '\n');
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
                Graph **temp = (Graph**)realloc(graphs, capacity * sizeof(Graph*));
                if (!temp) {
                    printf("Błąd: Nie udało się zaalokować pamięci dla tablicy grafów\n");
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
        Graph **temp = (Graph**)realloc(graphs, (*num_graphs) * sizeof(Graph*));
        if (temp) {
            graphs = temp;
        }
    }
    
    return graphs;
}


void free_memory(Graph *graph)
{
    if (graph) {
        if (graph->edge_groups) {
            for (int i = 0; i < graph->num_vertices; i++) {
                if (graph->edge_groups[i]) { //sprawdza czy nie null
                    free(graph->edge_groups[i]);
                }
            }
            free(graph->edge_groups);
        }
        if (graph->group_sizes) free(graph->group_sizes);
        if (graph->group_ptr) free(graph->group_ptr);
        if (graph->row) free(graph->row);
        if (graph->col) free(graph->col);
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
        printf("Błąd: Nie można otworyć pliku '%s'\n", filename);
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
    printf("Udało się zapisać wynik partycjonowania w pliku tekstowym '%s'.\n", filename);
}

void save_in_binary_file(PartitionResult *result, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Błąd: Nie można otworzyć pliku '%s'.\n", filename);
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
    printf("Udało się zapisać wynik partycjonowania w pliku binarnym '%s'.\n", filename);
}
    
void save_in_csrrg_format(PartitionResult *result, Graph *graph, char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Błąd: Nie można otworzyć pliku '%s'\n", filename);
        return;
    }
    //oryginalny graf
    // linia 1
    fprintf(file, "%d\n", graph->max_row_nodes);
    
    // linia 2
    for (int i = 0; i < graph->num_vertices; i++) {
        fprintf(file, "%d%s", graph->col[i], i < graph->num_vertices - 1 ? ";" : "");
    }
    fprintf(file, "\n");
    
    // linia 3
    int current_row = -1;
    for (int i = 0; i < graph->num_vertices; i++) {
        if (graph->row[i] != current_row) {
            fprintf(file, "%d;", i); 
            current_row = graph->row[i];
        }
    }
    if (graph->num_vertices > 0) {
        fprintf(file, "%d", graph->num_vertices);
    }
    fprintf(file, "\n");
    
    //linia 4
    for (int i = 0; i <= graph->num_groups + 1; i++) {
        if (graph->group_sizes[i] > 0) {
            fprintf(file, "%d", i);
            for (int j = 0; j < graph->group_sizes[i]; j++) {
                fprintf(file, ";%d", graph->edge_groups[i][j]);
            }
            if (i != graph->num_groups + 1) fprintf(file, ";");
        }
    }
    fprintf(file, "\n");
    
    //linia 5
    for (int i = 0; i < graph->num_groups; i++) {
        fprintf(file, "%d%s", graph->group_ptr[i], i < graph->num_groups - 1 ? ";" : "");
    }
    fprintf(file, "\n\n");

    //graf kazdej partycji
    for (int part = 0; part < result->num_parts; part++) {
        fprintf(file, "# Start of partition %d\n", part);
        fprintf(file, "%d\n", graph->max_row_nodes);
        
        // mapowanie wierzcholkow do nowych id
        int *vertex_mapping = (int*)malloc(graph->num_vertices * sizeof(int));
        for (int i = 0; i < graph->num_vertices; i++) {
            vertex_mapping[i] = -1;
        }
        
        int new_id = 0;
        for (int i = 0; i < graph->num_vertices; i++) {
            if (result->partition[i] == part) {
                vertex_mapping[i] = new_id++;
            }
        }
        
        // linia 2 (partycji)
        int first = 1;
        for (int i = 0; i < graph->num_vertices; i++) {
            if (result->partition[i] == part) {
                fprintf(file, "%s%d", first ? "" : ";", graph->col[i]);
                first = 0;
            }
        }
        fprintf(file, "\n");
        
        // linia 3 (partycji)
        fprintf(file, "0");
        current_row = -1;
        for (int i = 0; i < graph->num_vertices; i++) {
            if (result->partition[i] == part && graph->row[i] != current_row) {
                fprintf(file, ";%d", vertex_mapping[i]);
                current_row = graph->row[i];
            }
        }
        fprintf(file, "\n");
        
        //linia 4 (partycji)
        int group_count = 0;
        int *new_group_ptr = (int*)calloc(result->part_sizes[part] + 1, sizeof(int));
        new_group_ptr[0] = 0;
        
        for (int i = 0; i < graph->num_vertices; i++) {
            if (result->partition[i] != part) continue;
            
            int valid_edges = 0;
            for (int j = 0; j < graph->group_sizes[i]; j++) {
                if (result->partition[graph->edge_groups[i][j]] == part) {
                    valid_edges++;
                }
            }
            
            if (valid_edges > 0) {
                fprintf(file, "%d", vertex_mapping[i]);
                for (int j = 0; j < graph->group_sizes[i]; j++) {
                    int neighbor = graph->edge_groups[i][j];
                    if (result->partition[neighbor] == part) {
                        fprintf(file, ";%d", vertex_mapping[neighbor]);
                    }
                }
                fprintf(file, ";");
                
                group_count++;
                new_group_ptr[group_count] = new_group_ptr[group_count-1] + valid_edges + 1;
            }
        }
        fprintf(file, "\n");
        
        // linia 5 (partycji)
        for (int i = 0; i < group_count; i++) {
            fprintf(file, "%d%s", new_group_ptr[i], i < group_count - 1 ? ";" : "");
        }
        fprintf(file, "\n");
        
        free(vertex_mapping);
        free(new_group_ptr);
        fprintf(file, "# End of partition %d\n\n", part);
    }
    //stats
    fprintf(file, "# Statystyki partycjonowania:\n");
    fprintf(file, "# Liczba krawędzi przeciętych: %d\n", result->cut_edges);
    fprintf(file, "# Współczynnik nierównowagi: %.5f\n", result->imbalance);
    
    fclose(file);
    printf("Udało się zapisać wynik partycjonowania w pliku CSRRG '%s'.\n", filename);
}

