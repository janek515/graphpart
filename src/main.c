#include "args_parser.h"
#include "graph.h"
#include "io_handler.h"
#include "log_utils.h"
#include "partitioner.h"

#include <stdlib.h>

int main(int argc, char **argv) {
    Config config;
    if (!parse_args(argc, argv, &config)) {
        return 1;
    }

    if (config.verbose) {
        print_config(&config);
    }
    srand(config.seed);

    int graph_count = 0;
    Graph **graphs = read_multiple_graphs(config.input_filename, &graph_count);

    int num_parts = config.num_parts;
    float max_imbalance = config.max_imbalance;
    int num_attempts = config.num_attempts;
    int graph_index = config.graph_index;

    if (graph_index >= graph_count) {
        if (graph_count == 1) {
            error("Graf o indeksie %d nie istnieje. Jedyny dostępny "
                  "graf ma "
                  "indeks %d.\n",
                  graph_index, graph_count - 1);
        } else {
            error("Graf o indeksie %d nie istnieje. Zakres indeksów to "
                  "(0-%d).\n",
                  graph_index, graph_count - 1);
        }
        free_multiple_graphs(graphs, graph_count);
        return 1;
    }
    print_graph_details(graphs[graph_index]);
    if (config.num_attempts > 10000 && graphs[graph_index]->num_vertices > 10000) {
        warn(" Graf jest bardzo duży, przy podanej liczbie powtórzeń "
            "przetwarzanie może zająć dużo czasu.\n");
    }

    PartitionResult *result =
        spectral_partition(graphs[graph_index], num_parts, max_imbalance, num_attempts);
    if (!result) {
        free_multiple_graphs(graphs, graph_count);
        return 1;
    }

    if (config.output_format == FORMAT_BINARY) {
        save_in_binary_file(result, config.output_filename);
    } else {
        save_in_text_file(result, config.output_filename);
    }

    free_config(&config);
    free_partition_result(result);
    free_multiple_graphs(graphs, graph_count);
    return 0;
}
