#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int critical_set_reduce_graph(graph *g, node_id u, node_weight *offset,
                              buffers *b, change_list *c, reconstruction_data *d);

void critical_set_restore_graph(graph *g, reconstruction_data *d);

void critical_set_reconstruct_solution(int *I, reconstruction_data *d);

void critical_set_clean(reconstruction_data *d);

static reduction critical_set = {
    .reduce = critical_set_reduce_graph,
    .restore = critical_set_restore_graph,
    .reconstruct = critical_set_reconstruct_solution,
    .clean = critical_set_clean,
    .global = 1,
};
