#pragma once

#include "defs.h"
#include "graph.h"
#include "reduction_data.h"

/*
    Each reduction must define three functions:
        - reduce_graph
        - reconstruct_solution
        - clean

    See the definitions of each one for further information
*/

/*
    Try the reduction from vertex u
    if the reduction is successful, return 1, otherwise 0
    To run the reductions, the buffers in b can be used to avoid local allocations. Note that the fast_sets assume
    that no value larger than b->t is written anywhere, so DO NOT write larger values into these arrays.

    In the case of a successful reduction, the graph should be updated accordingly (see graph.h for helper functions)
    Additionally:
        - the offset should be written to the offset parameter
        - the c->n should be set to the number of vertices that observed a change in their neighborhood
        - the c->V should contain these vertices
        - the reconstruction_data should contain all necessary information to reconstruct the graph and solution.
          If the provided variables are not enough, allocate additional data and store the pointer in d->data.
          Note that any memory allocated should be freed in the clean function
*/
typedef int (*func_reduce_graph)(graph *g, node_id u, node_weight *offset, buffers *b, changed_list *c, reconstruction_data *d);

/*
    Only applicable when the reduction succeeded.
    This should lift the solutino from the reduced graph to the original.
    You can assume I is the size of the reduced graph, and contains a solution
    for the reduced graph.
*/
typedef void (*func_reconstruct_solution)(int *I, reconstruction_data *d);

/*
    Only applicable when the reduction succeeded.
    Cleanup, this will be called to free any allocated and placed in d->data
*/
typedef void (*func_clean)(reconstruction_data *d);

typedef struct
{
    func_reduce_graph reduce;
    func_reconstruct_solution reconstruct;
    func_clean clean;
    int global;
    const char *name;
} reduction;
