// #pragma once

// #include "defs.h"
// #include "reduction.h"
// #include "graph.h"

// int simultaneous_set_reduce_graph(graph *g, node_id u, node_weight *offset,
//                                   buffers *b, change_list *c, reconstruction_data *d);

// void simultaneous_set_restore_graph(graph *g, reconstruction_data *d);

// void simultaneous_set_reconstruct_solution(int *I, reconstruction_data *d);

// void simultaneous_set_clean(reconstruction_data *d);

// static reduction simultaneous_set = {
//     .reduce = simultaneous_set_reduce_graph,
//     .restore = simultaneous_set_restore_graph,
//     .reconstruct = simultaneous_set_reconstruct_solution,
//     .clean = simultaneous_set_clean,
//     .global = 0,
// };
