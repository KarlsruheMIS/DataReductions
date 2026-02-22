#pragma once

#include "defs.h"
#include "reduction.h"
#include "graph.h"

int extended_domination_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, changed_list *c, reconstruction_data *d);

void extended_domination_reconstruct_solution(int *I, reconstruction_data *d);

void extended_domination_clean(reconstruction_data *d);

static const reduction extended_domination = {
    .reduce = extended_domination_reduce_graph,
    .reconstruct = extended_domination_reconstruct_solution,
    .clean = extended_domination_clean,
    .global = 0,
    .name = "extended_domination"
};
