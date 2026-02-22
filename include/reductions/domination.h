#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int domination_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, changed_list *c, reconstruction_data *d);

void domination_reconstruct_solution(int *I, reconstruction_data *d);

void domination_clean(reconstruction_data *d);

static const reduction domination = {
    .reduce = domination_reduce_graph,
    .reconstruct = domination_reconstruct_solution,
    .clean = domination_clean,
    .global = 0,
    .name = "domination"
};
