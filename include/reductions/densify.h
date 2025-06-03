#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int densify_reduce_graph(graph *g, node_id u, node_weight *offset,
                         buffers *b, change_list *c, reconstruction_data *d);

void densify_restore_graph(graph *g, reconstruction_data *d);

void densify_reconstruct_solution(int *I, reconstruction_data *d);

void densify_clean(reconstruction_data *d);

static reduction densify = {
    .reduce = densify_reduce_graph,
    .restore = densify_restore_graph,
    .reconstruct = densify_reconstruct_solution,
    .clean = densify_clean,
    .global = 0,
};
