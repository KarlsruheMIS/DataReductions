#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int neighborhood_removal_reduce_graph(graph *g, node_id u, node_weight *offset,
                                      buffers *b, changed_list *c, reconstruction_data *d);

void neighborhood_removal_reconstruct_solution(int *I, reconstruction_data *d);

void neighborhood_removal_clean(reconstruction_data *d);

static const reduction neighborhood_removal = {
    .reduce = neighborhood_removal_reduce_graph,
    .reconstruct = neighborhood_removal_reconstruct_solution,
    .clean = neighborhood_removal_clean,
    .global = 0,
    .name = "neighborhood"
};
