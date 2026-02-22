#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int v_shape_reduce_graph(graph *g, node_id u, node_weight *offset,
                         buffers *b, changed_list *c, reconstruction_data *d);

void v_shape_reconstruct_solution(int *I, reconstruction_data *d);

void v_shape_clean(reconstruction_data *d);

static const reduction v_shape = {
    .reduce = v_shape_reduce_graph,
    .reconstruct = v_shape_reconstruct_solution,
    .clean = v_shape_clean,
    .global = 0,
    .name = "v_shape"
};
