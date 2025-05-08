#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int degree_zero_reduce_graph(graph *g, node_id u, node_weight *offset,
                             buffers *b, change_list *c, reconstruction_data *d);

void degree_zero_restore_graph(graph *g, reconstruction_data *d);

void degree_zero_reconstruct_solution(int *I, reconstruction_data *d);

void degree_zero_clean(reconstruction_data *d);

static reduction degree_zero = {
    .reduce = degree_zero_reduce_graph,
    .restore = degree_zero_restore_graph,
    .reconstruct = degree_zero_reconstruct_solution,
    .clean = degree_zero_clean,
    .global = 0,
};
