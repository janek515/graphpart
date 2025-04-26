#include "args_parser.h"
#include "log_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// wartosci domyslne
#define DEFAULT_OUTPUT_FILE "output"
#define DEFAULT_PARTS 2
#define DEFAULT_MAX_IMBALANCE 1.10f
#define DEFAULT_GRAPH_INDEX 0
#define DEFAULT_FORMAT FORMAT_TEXT
#define DEFAULT_NUM_ATTEMPTS 10

void init_config(Config *config) {
    if (!config) {
        return;
    }

    config->input_filename = NULL;
    config->output_filename = strdup(DEFAULT_OUTPUT_FILE);
    config->num_parts = DEFAULT_PARTS;
    config->max_imbalance = DEFAULT_MAX_IMBALANCE;
    config->graph_index = DEFAULT_GRAPH_INDEX;
    config->output_format = DEFAULT_FORMAT;
    config->verbose = 0;
    config->seed = (unsigned int)time(NULL);
    config->num_attempts = DEFAULT_NUM_ATTEMPTS;
}

void free_config(Config *config) {
    if (!config) {
        return;
    }

    free(config->input_filename);
    free(config->output_filename);
}

char *get_file_extension(char *filename) {
    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return "";
    }
    return dot + 1;
}

char *ensure_extension(char *filename, char *required_ext) {
    char *ext = strrchr(filename, '.');

    // jesli juz poprawne rozszerz
    if (ext && strcmp(ext + 1, required_ext) == 0) {
        return strdup(filename);
    }
    char *base_name = strdup(filename);
    char *dot = strrchr(base_name, '.');

    // nowe rozszerz
    if (dot) {
        *dot = '\0';
    }

    char *new_filename = malloc(strlen(base_name) + strlen(required_ext) + 2);
    sprintf(new_filename, "%s.%s", base_name, required_ext);

    free(base_name);
    return new_filename;
}

int parse_args(int argc, char *argv[], Config *config) {
    if (argc < 2 || !config) {
        info("Witamy w graphpart! Wpisz %s --help aby zobaczyć dostępne "
             "opcje.\n",
             argv[0]);
        return 0;
    }

    init_config(config);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0) {
            if (++i < argc) {
                char *input_ext = get_file_extension(argv[i]);
                if (strcmp(input_ext, "csrrg") != 0) {
                    error("Niepoprawny format pliku wejściowego. Użyj "
                          "formatu "
                          "csrrg.\n");
                    free_config(config);
                    return 0;
                }
                config->input_filename = strdup(argv[i]);
            } else {
                error("Brakuje nazwy pliku wejściowego.\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--output") == 0) {
            if (++i < argc) {
                free(config->output_filename);
                config->output_filename = strdup(argv[i]);
                char *ext = (config->output_format == FORMAT_BINARY)
                                ? "bin"
                                : ((config->output_format == FORMAT_TEXT) ? "txt" : "csrrg");
                warn("Podano rozszerzenie niezgodne z podanym formatem albo nie "
                     "podano formatu i użyto domyślnego (text). Zmieniam "
                     "rozszerzenie na poprawne.\n");
                config->output_filename = ensure_extension(argv[i], ext);
            } else {
                error("Brakuje nazwy pliku wyjściowego.\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--format") == 0) {
            if (++i < argc) {
                if (strcmp(argv[i], "text") == 0) {
                    config->output_format = FORMAT_TEXT;
                    if (config->output_filename) {
                        char *new_filename = ensure_extension(config->output_filename, "txt");
                        free(config->output_filename);
                        config->output_filename = new_filename;
                    }
                } else if (strcmp(argv[i], "binary") == 0) {
                    config->output_format = FORMAT_BINARY;
                    if (config->output_filename) {
                        char *new_filename = ensure_extension(config->output_filename, "bin");
                        free(config->output_filename);
                        config->output_filename = new_filename;
                    }
                } else if (strcmp(argv[i], "csrrg") == 0) {
                    config->output_format = FORMAT_CSRRG;
                    if (config->output_filename) {
                        char *new_filename = ensure_extension(config->output_filename, "csrrg");
                        free(config->output_filename);
                        config->output_filename = new_filename;
                    }
                } else {
                    error("Niepoprawny format. Wpisz 'text' lub "
                          "'binary'.\n");
                    return 0;
                }
            } else {
                error("Brakuje specyfikacji formatu.\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--parts") == 0) {
            if (++i < argc) {
                int parts = atoi(argv[i]);
                if (parts <= 1) {
                    error("Liczba partycji grafu musi być liczbą całkowitą "
                          "większą od 1.\n");
                    return 0;
                }
                config->num_parts = parts;
            } else {
                error("Brakuje wartości liczby partycji grafu.\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--max-imbalance") == 0) {
            if (++i < argc) {
                float imbalance = atof(argv[i]);
                if (imbalance < 1.0f) {
                    error("Maksymalny współczynnik nierównowagi musi "
                          "być większy "
                          "lub równy 1.0.\n");
                    return 0;
                }
                config->max_imbalance = imbalance;
            } else {
                error("Brakuje wartości maksymalnego współczynnika "
                      "nierównowagi.\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--graph-index") == 0) {
            if (++i < argc) {
                int index = atoi(argv[i]);
                if (index < 0) {
                    error("Indeks grafu musi być liczbą całkowitą "
                          "większą lub "
                          "równą 0.\n");
                    return 0;
                }
                config->graph_index = index;
            } else {
                error("Brakuje wartości indeksu grafu.\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--attempts") == 0) {
            if (++i < argc) {
                int attempts = atoi(argv[i]);
                if (attempts < 1) {
                    error("Liczba powtórzeń musi być liczbą całkowitą "
                          "większą lub "
                          "równą 1. U\n");
                    return 0;
                }
                config->num_attempts = attempts;
            } else {
                error("Brakuje wartości liczby powtórzeń.\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--seed") == 0) {
            if (++i < argc) {
                unsigned int seed = (unsigned int)atoi(argv[i]);
                config->seed = seed;
            } else {
                error("Brakuje wartości ziarna.\n");
                return 0;
            }
        } else if (strcmp(argv[i], "--verbose") == 0) {
            config->verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Witamy w graphpart, programie do dzielenia grafów przy użyciu "
                   "algorytmu k-średnich.\n");
            printf("Program przyjmuje plik z grafem w formacie csrrg, traktując "
                   "zawarty w nim graf jako nieskierowany.\n");
            printf("Wynik partycjonowania jest zapisywany w pliku tekstowym lub "
                   "binarnym.\n");
            printf("\n");
            printf("Użycie programu: %s [opcje]\n", argv[0]);
            printf("Opcje:\n");
            printf("  --input <filename>\n");
            printf("        Nazwa pilku wejściowego w formacie csrrg "
                   "[wymagane]\n");
            printf("\n");
            printf("  --format <text|binary|csrrg>\n");
            printf("        Format wyjściowy [domyślnie: text]\n");
            printf("\n");
            printf("  --output <filename>\n");
            printf("        Nazwa pliku wyjściowego [domyślnie: 'output.txt']\n");
            printf("\n");
            printf("  --parts <number>\n");
            printf("        Liczba partycji grafu; wartość będąca liczbą "
                   "całkowitą > "
                   "1 [domyślnie: 2]\n");
            printf("\n");
            printf("  --max-imbalance <value>\n");
            printf("        Maksymalny dozwolony współczynnik nierównowagi między "
                   "partycjami; wartość będąca ułamkiem dziesiętnym >= 1.0 "
                   "[domyślnie: 1.10]\n");
            printf("\n");
            printf("  --graph-index <index>\n");
            printf("        Indeks grafu do przetworzenia, jeśli plik zawiera "
                   "więcej "
                   "niż jeden graf; indeksowanie rozpoczyna się od 0 "
                   "[domyślnie: 0]\n");
            printf("\n");
            printf("  --attempts <number>\n");
            printf("        Liczba powtórzeń algorytmu w celu uzyskania najlepszego możliwego "
                   "wyniku partycjonowania [domyślnie: 10]\n");
            printf("\n");
            printf("  --seed <number>\n");
            printf("        Ziarno losowości w algorytmie k-średnich [domyślnie: aktualny "
                   "timestamp]\n");
            printf("\n");
            printf("  --verbose\n");
            printf("        Włącza tryb szczegółowego wypisywania informacji o "
                   "przebiegu procesu partycjonowania\n");
            printf("\n");
            printf("  --help\n");
            printf("        Wyświetla pomoc dotyczącą użycia programu\n");
            printf("\n");
            return 0;
        } else {
            error("Nieznany argument '%s'. Wpisz %s --help, aby wyświetlić "
                  "pomoc.\n",
                  argv[i], argv[0]);
            return 0;
        }
    }

    return 1;
}

void print_config(Config *config) {
    printf("\n");
    verbose("Konfiguracja programu:\n");
    verbose("Plik wejściowy:         %s\n", config->input_filename);
    verbose("Format wyjściowy:       %s\n",
            config->output_format == FORMAT_TEXT ? "text" : "binary");
    verbose("Plik wyjściowy:         %s\n", config->output_filename);
    verbose("Liczba partycji:        %d\n", config->num_parts);
    verbose("Max. nierównowaga:      %.2f\n", config->max_imbalance);
    verbose("Indeks grafu:           %d\n", config->graph_index);
    verbose("Liczba powtórzeń:       %d\n\n", config->num_attempts);
}
