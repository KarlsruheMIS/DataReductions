#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int simplicial_vertex_with_weight_transfer_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, change_list *c, reconstruction_data *d);

void simplicial_vertex_with_weight_transfer_restore_graph(graph *g, reconstruction_data *d);

void simplicial_vertex_with_weight_transfer_reconstruct_solution(int *I, reconstruction_data *d);

void simplicial_vertex_with_weight_transfer_clean(reconstruction_data *d);

static reduction simplicial_vertex_with_weight_transfer = {
    .reduce = simplicial_vertex_with_weight_transfer_reduce_graph,
    .restore = simplicial_vertex_with_weight_transfer_restore_graph,
    .reconstruct = simplicial_vertex_with_weight_transfer_reconstruct_solution,
    .clean = simplicial_vertex_with_weight_transfer_clean,
    .global = 0,
};
