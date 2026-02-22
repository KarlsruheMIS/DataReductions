#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int struction_reduce_graph(graph *g, node_id u, node_weight *offset,
                           buffers *b, changed_list *c, reconstruction_data *d);

void struction_reconstruct_solution(int *I, reconstruction_data *d);

void struction_clean(reconstruction_data *d);

static const reduction struction = {
    .reduce = struction_reduce_graph,
    .reconstruct = struction_reconstruct_solution,
    .clean = struction_clean,
    .global = 0,
    .name = "struction"
};
