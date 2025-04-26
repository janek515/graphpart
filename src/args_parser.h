#ifndef ARGS_PARSER_H
#define ARGS_PARSER_H

typedef enum { FORMAT_TEXT, FORMAT_BINARY, FORMAT_CSRRG } OutputFormat;

typedef struct {
    char *input_filename;       // Nazwa pliku wejsciowego
    char *output_filename;      // Nazwa pliku wyjsciowego
    int num_parts;              // Liczba partycji
    float max_imbalance;        // Maksymalny wspolczynnik nierownowagl
    int graph_index;            // Indeks grafu (jesli jest wiele grafow)
    OutputFormat output_format; // Format wyjsciowy (text/binary)
    int verbose;                // Tryb szczegolowego wypisywania
    unsigned int seed;          // Ziarno losowosci (opcjonalne)
    int num_attempts;           // Liczba prob (opcjonalne)
} Config;

int parse_args(int argc, char *argv[], Config *config);
void print_config(Config *config);
void free_config(Config *config);

#endif
