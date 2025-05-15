#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int extended_struction_reduce_graph(graph *g, node_id u, node_weight *offset,
                                    buffers *b, change_list *c, reconstruction_data *d);

void extended_struction_restore_graph(graph *g, reconstruction_data *d);

void extended_struction_reconstruct_solution(int *I, reconstruction_data *d);

void extended_struction_clean(reconstruction_data *d);

static reduction extended_struction = {
    .reduce = extended_struction_reduce_graph,
    .restore = extended_struction_restore_graph,
    .reconstruct = extended_struction_reconstruct_solution,
    .clean = extended_struction_clean,
    .global = 0,
};
