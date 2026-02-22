#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int weighted_funnel_reduce_graph(graph *g, node_id u, node_weight *offset,
                                 buffers *b, changed_list *c, reconstruction_data *d);

void weighted_funnel_reconstruct_solution(int *I, reconstruction_data *d);

void weighted_funnel_clean(reconstruction_data *d);

static const reduction weighted_funnel = {
    .reduce = weighted_funnel_reduce_graph,
    .reconstruct = weighted_funnel_reconstruct_solution,
    .clean = weighted_funnel_clean,
    .global = 0,
    .name = "funnel"
};
