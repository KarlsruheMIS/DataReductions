#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int degree_one_reduce_graph(graph *g, node_id u, node_weight *offset, buffers *b, change_list *c, reconstruction_data *d);

void degree_one_restore_graph(graph *g, reconstruction_data *d);

void degree_one_reconstruct_solution(int *I, reconstruction_data *d);

void degree_one_clean(reconstruction_data *d);

static reduction degree_one = {
    .reduce = degree_one_reduce_graph,
    .restore = degree_one_restore_graph,
    .reconstruct = degree_one_reconstruct_solution,
    .clean = degree_one_clean,
    .global = 0
};
