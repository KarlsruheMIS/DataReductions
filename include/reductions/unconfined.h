#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int unconfined_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, change_list *c, reconstruction_data *d);

void unconfined_restore_graph(graph *g, reconstruction_data *d);

void unconfined_reconstruct_solution(int *I, reconstruction_data *d);

void unconfined_clean(reconstruction_data *d);

static reduction unconfined = {
    .reduce = unconfined_reduce_graph,
    .restore = unconfined_restore_graph,
    .reconstruct = unconfined_reconstruct_solution,
    .clean = unconfined_clean,
    .global = 0,
};
