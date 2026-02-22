#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int twin_reduce_graph(graph *g, node_id u, node_weight *offset,
                      buffers *b, changed_list *c, reconstruction_data *d);

void twin_reconstruct_solution(int *I, reconstruction_data *d);

void twin_clean(reconstruction_data *d);

static const reduction twin = {
    .reduce = twin_reduce_graph,
    .reconstruct = twin_reconstruct_solution,
    .clean = twin_clean,
    .global = 0,
    .name = "twin"
};
