#include "extended_struction.h"
#include "algorithms.h"

#include <assert.h>
#include <stdlib.h>

typedef struct
{
    node_id n, m;
    node_id *V, *E;
} extended_struction;

extended_struction *extended_struction_init()
{
    extended_struction *es = malloc(sizeof(extended_struction));

    es->n = 0;
    es->m = 0;

    es->V = malloc(sizeof(node_id) * (MAX_STRUCTION + 1));
    es->E = malloc(sizeof(node_id) * MAX_STRUCTION * MAX_STRUCTION);

    return es;
}

int extended_struction_reduce_graph(graph *g, node_id u, node_weight *offset,
                                    buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > MAX_STRUCTION)
        return 0;

    return 0;
}

void extended_struction_restore_graph(graph *g, reconstruction_data *d)
{
}

void extended_struction_reconstruct_solution(int *I, reconstruction_data *d)
{
}

void extended_struction_clean(reconstruction_data *d)
{
}