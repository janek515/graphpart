#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "graph.h"
#define MAX 1000000

int get_last_value(char *buffer) {

    int len = strlen(buffer);
    if (len == 0) {
        printf("Error: Empty buffer\n");
        return 0;  // Return 0 for empty buffer
    }
    // Remove trailing characters (newline, semicolon, space)
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ';' || buffer[len - 1] == ' ')) {
        buffer[--len] = '\0';
    }

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

// Helper function to compare integers for qsort
int compare_ints(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
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

    char buffer[MAX];  // line buffer 
    char line4_copy[MAX];   
    char line5_copy[MAX]; 

    // max vertices/nodes per row
    fscanf(file, "%d\n", &graph->max_row_nodes);

    // currently dont need line 2 for anything, later can be used to get information on what column each vertex is in
    fgets(buffer, sizeof(buffer), file);
    //int n_cols = get_max_value(buffer);  // number of columns, not used rn?

    // line 3: vertex count, later can be used to get information on what row each vertex is in  
    fgets(buffer, sizeof(buffer), file);
    graph->num_vertices = get_last_value(buffer);


    // edge_groups - skip line 4 (store for later)
    fgets(buffer, sizeof(buffer), file);
    //count elements in line 4
    int count = 0;
    for (char *p = buffer; *p; p++) {
        if (*p == ';') count++;
    }
    count++;
    //printf("Debug: num of elements in line 4 = %d\n", count); 
    strcpy(line4_copy, buffer);

    // group_pointers
    fgets(buffer, sizeof(buffer), file);
    strcpy(line5_copy, buffer); 

    // count groups
    int group_count = 1;  // start with 1 for the last group
    for(char *p = buffer; *p; p++) {
        if(*p == ';') group_count++;
    }
    graph->num_groups = group_count;
    //printf("Debug: num_groups = %d\n", graph->num_groups);

    // Allocate memory for group_ptr
    graph->group_ptr = (int*)malloc((graph->num_groups) * sizeof(int));
    if (!graph->group_ptr) {
        printf("Error: Failed to allocate memory for group_ptr\n");
        free(graph);
        fclose(file);
        return NULL;
    }

    // Parse group pointers
    char *token = strtok(line5_copy, ";");
    int group_idx = 0;
    while (token != NULL && group_idx < graph->num_groups) {
        //printf("Debug: group_ptr[%d] = %s\n", group_idx, token);
        graph->group_ptr[group_idx++] = atoi(token);
        token = strtok(NULL, ";");
    }

    // Allocate and parse edge groups
    graph->edge_groups = (int**)malloc(graph->num_vertices * sizeof(int*));
    if (!graph->edge_groups) {
        printf("Error: Failed to allocate memory for edge_groups\n");
        free(graph->group_ptr);
        free(graph);
        fclose(file);
        return NULL;
    }

    // Allocate memory for storing the size of each edge_num_group
    graph->group_sizes = (int*)malloc(graph->num_vertices * sizeof(int));
    if (!graph->group_sizes) {
        printf("Error: Failed to allocate memory for group_sizes\n");
        free(graph->edge_groups);
        free(graph->group_ptr);
        free(graph);
        fclose(file);
        return NULL;
    }

    token = strtok(line4_copy, ";");
    graph->num_edges = 0; // Initialize the total number of edges
    int k = 0;  // Separate index for group_ptr

    for (int i = 0; i < graph->num_vertices; i++) {
        if (k < graph->num_groups && atoi(token) == i) { // Check if the first element matches the vertex index
            int start_idx = (k < graph->num_groups) ? graph->group_ptr[k] : 0;
            int end_idx = (k < graph->num_groups - 1) ? graph->group_ptr[k + 1] : count;

            // Calculate the size of the edge group
            graph->group_sizes[i] = (end_idx > start_idx) ? (end_idx - start_idx - 1) : 0;
            graph->num_edges += graph->group_sizes[i];

            // Allocate memory for the edge group
            if (graph->group_sizes[i] > 0) {
                graph->edge_groups[i] = (int*)malloc(graph->group_sizes[i] * sizeof(int));
                if (!graph->edge_groups[i]) {
                    printf("Error: Failed to allocate memory for edge_groups[%d]\n", i);
                    for (int j = 0; j < i; j++) {
                        if (graph->edge_groups[j]) {
                            free(graph->edge_groups[j]);
                        }
                    }
                    free(graph->edge_groups);
                    free(graph->group_ptr);
                    free(graph);
                    fclose(file);
                    return NULL;
                }
            } else {
                graph->edge_groups[i] = NULL;  // Explicitly set NULL for safety
            }

            // Populate the group (skip the first element)
            token = strtok(NULL, ";"); // Skip the first element
            for (int j = 0; j < graph->group_sizes[i]; j++) {
                if (token == NULL) {
                    printf("Error: Unexpected end of line4_copy while parsing edge_groups\n");
                    for (int k = 0; k <= i; k++) {
                        if (graph->edge_groups[k]) {
                            free(graph->edge_groups[k]);
                        }
                    }
                    free(graph->edge_groups);
                    free(graph->group_ptr);
                    free(graph);
                    fclose(file);
                    return NULL;
                }
                graph->edge_groups[i][j] = atoi(token);
                token = strtok(NULL, ";");
            }

            // Sort the group (excluding the vertex index)
            qsort(graph->edge_groups[i], graph->group_sizes[i], sizeof(int), compare_ints);

            // Debug output for the size and elements of each edge_group
            /*printf("Debug: (after population) edge_groups[%d] (size %d) = ", i, graph->group_sizes[i]);
            for (int j = 0; j < graph->group_sizes[i]; j++) {
                printf("%d ", graph->edge_groups[i][j]);
            }
            printf("\n");*/

            k++; // Increment group pointer index
        } else {
            // If no group matches, set size to 0 and edge_groups[i] to NULL
            graph->group_sizes[i] = 0;
            graph->edge_groups[i] = NULL;
        }
    }

    // Debug output for total number of edges
    //printf("Debug: Total number of edges = %d\n", graph->num_edges);
    /* printf("graph edge groups: \n");
    for (int i = 0; i < graph->num_vertices; i++) {
        for (int j = 0; j < graph->group_sizes[i]; j++) {
            printf("%d; ", graph->edge_groups[i][j]);
        }
    }*/

    fclose(file);
    return graph;
}


void free_memory(Graph *graph)
{
    if (graph) {
        if (graph->edge_groups) {
            for (int i = 0; i < graph->num_vertices; i++) {
                if (graph->edge_groups[i]) {  // Check for NULL
                    free(graph->edge_groups[i]);
                }
            }
            free(graph->edge_groups);
        }
        if (graph->group_sizes) free(graph->group_sizes);
        if (graph->group_ptr) free(graph->group_ptr);
        free(graph);
    }
}
