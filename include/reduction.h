#pragma once

#include "defs.h"
#include "graph.h"

/*
    Each reduction must define four functions:
        - reduce_graph
        - restore_graph
        - reconstruct_solution
        - clean

    See the definitions of each one for further information
*/

typedef struct
{
    long long _a; // Number of elements allocated for (should always be >= to n)

    // Buffers
    node_id **buffers;

    // Fast sets
    int t;
    int **fast_sets;
} buffers;

typedef struct
{
    long long _a; // Number of elements allocated for (should always be >= to n)

    node_id n;  // Number of vertices changed
    node_id *V; // The changed vertices
    int *in_V;  // Set to 1 if vertex is in the list
} change_list;

typedef struct
{
    // Commonly used variables for most reductions
    node_id u, v, w, x, y, z, n;
    node_id e1, e2, e3, e4;
    void *data; // For additional data (must be allocated manually)
} reconstruction_data;

/*
    Try the reduction from vertex u
    if the reduction is successful, return 1, otherwise 0
    To test the reductions, the buffers in b can be used to avoid local allocations. Note that the fast_sets assume
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
typedef int (*func_reduce_graph)(graph *g, node_id u, node_weight *offset, buffers *b, change_list *c, reconstruction_data *d);

/*
    Only applicable when the reduction succeeded.
    This should restore the graph to how it was before the reduction.
    Note that the deactivate/activate functions are sufficient in most cases
 */
typedef void (*func_restore_graph)(graph *g, reconstruction_data *d);

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
    func_restore_graph restore;
    func_reconstruct_solution reconstruct;
    func_clean clean;
    int global;
} reduction;

void reduction_data_init(graph *g, buffers **b, change_list **c);

void reduction_data_increase(buffers *b, change_list *c);

void reduction_data_free(buffers *b, change_list *c);

void reduction_data_reset_fast_sets(buffers *b);

void reduction_data_queue_distance_one(graph *g, node_id u, change_list *c);

void reduction_data_queue_distance_two(graph *g, node_id u, change_list *c);