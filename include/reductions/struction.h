#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

extern int STRUCTION_MAX_DEGREE;
extern int STRUCTION_MAX_NODES;

int struction_reduce_graph(graph *g, node_id u, node_weight *offset,
                           buffers *b, change_list *c, reconstruction_data *d);

void struction_restore_graph(graph *g, reconstruction_data *d);

void struction_reconstruct_solution(int *I, reconstruction_data *d);

void struction_clean(reconstruction_data *d);

static reduction struction = {
    .reduce = struction_reduce_graph,
    .restore = struction_restore_graph,
    .reconstruct = struction_reconstruct_solution,
    .clean = struction_clean,
    .global = 0,
};
