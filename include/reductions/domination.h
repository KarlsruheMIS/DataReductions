#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int domination_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, change_list *c, reconstruction_data *d);

void domination_restore_graph(graph *g, reconstruction_data *d);

void domination_reconstruct_solution(int *I, reconstruction_data *d);

void domination_clean(reconstruction_data *d);

static reduction domination = {
    .reduce = domination_reduce_graph,
    .restore = domination_restore_graph,
    .reconstruct = domination_reconstruct_solution,
    .clean = domination_clean,
    .global = 0,
};
