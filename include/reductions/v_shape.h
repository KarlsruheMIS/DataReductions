#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int v_shape_reduce_graph(graph *g, node_id u, node_weight *offset,
                         buffers *b, change_list *c, reconstruction_data *d);

void v_shape_restore_graph(graph *g, reconstruction_data *d);

void v_shape_reconstruct_solution(int *I, reconstruction_data *d);

void v_shape_clean(reconstruction_data *d);

static reduction v_shape = {
    .reduce = v_shape_reduce_graph,
    .restore = v_shape_restore_graph,
    .reconstruct = v_shape_reconstruct_solution,
    .clean = v_shape_clean,
    .global = 0,
};
