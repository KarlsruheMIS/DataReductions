#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int triangle_reduce_graph(graph *g, node_id u, node_weight *offset, buffers *b, change_list *c, reconstruction_data *d);

void triangle_restore_graph(graph *g, reconstruction_data *d);

void triangle_reconstruct_solution(int *I, reconstruction_data *d);

void triangle_clean(reconstruction_data *d);

static reduction triangle = {
    .reduce = triangle_reduce_graph,
    .restore = triangle_restore_graph,
    .reconstruct = triangle_reconstruct_solution,
    .clean = triangle_clean,
    .global = 0,
};
