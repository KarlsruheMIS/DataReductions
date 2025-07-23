#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int edge_expansion_reduce_graph(graph *g, node_id u, node_weight *offset,
                                buffers *b, change_list *c, reconstruction_data *d);

void edge_expansion_restore_graph(graph *g, reconstruction_data *d);

void edge_expansion_reconstruct_solution(int *I, reconstruction_data *d);

void edge_expansion_clean(reconstruction_data *d);

static reduction edge_expansion = {
    .reduce = edge_expansion_reduce_graph,
    .restore = edge_expansion_restore_graph,
    .reconstruct = edge_expansion_reconstruct_solution,
    .clean = edge_expansion_clean,
    .global = 0,
};
