#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int simplicial_vertex_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, change_list *c, reconstruction_data *d);

void simplicial_vertex_restore_graph(graph *g, reconstruction_data *d);

void simplicial_vertex_reconstruct_solution(int *I, reconstruction_data *d);

void simplicial_vertex_clean(reconstruction_data *d);

static reduction simplicial_vertex = {
    .reduce = simplicial_vertex_reduce_graph,
    .restore = simplicial_vertex_restore_graph,
    .reconstruct = simplicial_vertex_reconstruct_solution,
    .clean = simplicial_vertex_clean,
    .global = 0,
};
