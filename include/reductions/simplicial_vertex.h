#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int simplicial_vertex_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, changed_list *c, reconstruction_data *d);

void simplicial_vertex_reconstruct_solution(int *I, reconstruction_data *d);

void simplicial_vertex_clean(reconstruction_data *d);

static const reduction simplicial_vertex = {
    .reduce = simplicial_vertex_reduce_graph,
    .reconstruct = simplicial_vertex_reconstruct_solution,
    .clean = simplicial_vertex_clean,
    .global = 0,
    .name = "simplicial_vertex"
};
