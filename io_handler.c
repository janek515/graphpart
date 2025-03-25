#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "graph.h"
#define MAX 100000

int get_last_value(char *buffer) {
    // Debug print to see the raw buffer
    /*printf("Raw buffer: [%s]\n", buffer);

    printf("Cleaned buffer content (ASCII): ");
    for (int i = 0; buffer[i] != '\0'; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");*/

    int len = strlen(buffer);
    if (len == 0) {
        printf("Error: Empty buffer\n");
        return 0;  // Return 0 for empty buffer
    }
    // Remove trailing characters (newline, semicolon, space)
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ';' || buffer[len - 1] == ' ')) {
        buffer[--len] = '\0';
    }
    // Debug print to see the cleaned buffer
    /*printf("Cleaned buffer: [%s]\n", buffer);

    printf("Cleaned buffer content (ASCII): ");
    for (int i = 0; buffer[i] != '\0'; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");*/
    if (len == 0) {
        printf("Error: Buffer only contained trailing characters\n");
        return 0;  // Return 0 if the buffer is now empty
    }
    // Find the last number
    int end = len - 1;
    while (end >= 0 && (buffer[end] == ';' || buffer[end] == ' ')) {
        end--;  // Skip separators
    }
    int start = end;
    while (start >= 0 && buffer[start] != ';' && buffer[start] != ' ') {
        start--;  // Find the start of the last number
    }
    return atoi(buffer + start + 1);;
}

Graph* read_file(char *filename) {
  FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Błąd: Nie można otworzyć pliku '%s'\n", filename);
        exit(1);
    }

    Graph *graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) {
        printf("Błąd: Nie udało się zaalokować pamięci dla grafu\n");
        fclose(file);
        return NULL;
    }

    // max vertices/nodes per row
    fscanf(file, "%d\n", &graph->max_row_nodes);

    char buffer[MAX];  // line buffer
    char line2_copy[MAX];
    char line4_copy[MAX];   //line 4 copy
    char line5_copy[MAX];   //line 5 copy

    // skip line 2 (store for later)
    fgets(buffer, sizeof(buffer), file);
    strcpy(line2_copy, buffer);

    // read row pointers (line 3) to get vertex count
    fgets(buffer, sizeof(buffer), file);
    graph->num_vertices = get_last_value(buffer);

    // allocate and parse col_indices using the stored line
    graph->col_indices = (int*)malloc(graph->num_vertices * sizeof(int));
    if (!graph->col_indices) {
        printf("Błąd: Nie udało się zaalokować pamięci dla col_indices\n");
        free(graph);
        fclose(file);
        return NULL;
    }

    char *token = strtok(line2_copy, ";");
    int col_idx = 0;
    while (token != NULL && col_idx < graph->num_vertices) {
        graph->col_indices[col_idx++] = atoi(token);
        token = strtok(NULL, ";");
    }

    // continue with row pointers processing using the buffer we already read
    int row_pointers_count = 0;
    char *ptr = buffer;
    int temp;
    while (*ptr && sscanf(ptr, "%d;", &temp) == 1) {
        row_pointers_count++;
        while (*ptr && *ptr != ';')
            ptr++;
        if (*ptr == ';')
            ptr++;
    }
    
    graph->row_ptr = (int*)malloc(row_pointers_count * sizeof(int));
    if (!graph->row_ptr) {
        printf("Błąd: Nie udało się zaalokować pamięci dla row_pointers\n");
        free(graph);
        fclose(file);
        return NULL;
    }

    // parsing row pointers after counting them
    ptr = buffer;  // reset pointer to start of buffer
    int row_idx = 0;
    while (*ptr && sscanf(ptr, "%d;", &temp) == 1) {
        graph->row_ptr[row_idx++] = temp;
        while (*ptr && *ptr != ';') ptr++;
        if (*ptr == ';') ptr++;
    }

    // edge_groups - skip line 4 (store for later)
    fgets(buffer, sizeof(buffer), file);
    strcpy(line4_copy, buffer);

    // group_pointers
    fgets(buffer, sizeof(buffer), file);
    strcpy(line5_copy, buffer); 
    graph->num_edges = get_last_value(line5_copy);

    // count groups
    int group_count = 1;  // start with 1 for the last group
    for(char *p = buffer; *p; p++) {
        if(*p == ';') group_count++;
    }
    graph->num_groups = group_count;

    // allocate memory for group pointers
    graph->group_ptr = (int*)malloc(graph->num_groups * sizeof(int));
    if (!graph->group_ptr) {
        printf("Błąd: Nie udało się zaalokować pamięci dla group_pointers\n");
        free(graph->col_indices);
        free(graph->row_ptr);
        free(graph);
        fclose(file);
        return NULL;
    }

    // parse group pointers
    token = strtok(line5_copy, ";");
    int group_idx = 0;
    while (token != NULL && group_idx < graph->num_groups) {
        graph->group_ptr[group_idx++] = atoi(token);
        token = strtok(NULL, ";");
    }

    // allocate and parse edge groups
    graph->edge_groups = (int**)malloc(graph->num_groups * sizeof(int*));
    if (!graph->edge_groups) {
        printf("Błąd: Nie udało się zaalokować pamięci dla edge_groups\n");
        free(graph->group_ptr);
        free(graph->row_ptr);
        free(graph->col_indices);
        free(graph);
        fclose(file);
        return NULL;
    }

    token = strtok(line4_copy, ";");
    for (int i = 0; i < graph->num_groups && token != NULL; i++) {
        // count numbers in this group
        int nums_in_group = 0;
        char *num = token;
        while (*num) {
            if (isdigit(*num)) {
                nums_in_group++;
                while (isdigit(*num)) num++;
            } else {
                num++;
            }
        }

        // allocate memory for this group
        graph->edge_groups[i] = (int*)malloc(nums_in_group * sizeof(int));
        if (!graph->edge_groups[i]) {
            for (int i = 0; i < graph->num_groups; i++) {
                free(graph->edge_groups[i]);
            }
            free(graph->edge_groups);
            free(graph->group_ptr);
            free(graph->row_ptr);
            free(graph->col_indices);
            free(graph);
            fclose(file);
            return NULL;
        }

        // parse numbers in this group
        num = token;
        int j = 0;
        while (*num) {
            if (isdigit(*num)) {
                graph->edge_groups[i][j++] = atoi(num);
                while (isdigit(*num)) num++;
            } else {
                num++;
            }
        }
        token = strtok(NULL, ";");
    }

    fclose(file);
    return graph;
}


void free_memory(Graph *graph)
{
    for (int i = 0; i < graph->num_groups; i++) {
        free(graph->edge_groups[i]);
    }
    free(graph->edge_groups);
    free(graph->group_ptr);
    free(graph->row_ptr);
    free(graph->col_indices);
    free(graph);
}
