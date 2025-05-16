#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int weighted_funnel_reduce_graph(graph *g, node_id u, node_weight *offset,
                                 buffers *b, change_list *c, reconstruction_data *d);

void weighted_funnel_restore_graph(graph *g, reconstruction_data *d);

void weighted_funnel_reconstruct_solution(int *I, reconstruction_data *d);

void weighted_funnel_clean(reconstruction_data *d);

static reduction weighted_funnel = {
    .reduce = weighted_funnel_reduce_graph,
    .restore = weighted_funnel_restore_graph,
    .reconstruct = weighted_funnel_reconstruct_solution,
    .clean = weighted_funnel_clean,
    .global = 0,
};
