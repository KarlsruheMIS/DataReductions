#include "degree_zero.h"

#include <assert.h>

int degree_zero_reduce_graph(graph *g, node_id u, node_weight *offset,
                             buffers *b, changed_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > 0)
        return 0;

    *offset = g->W[u];
    d->u = u;
    
    graph_remove_vertex(g, u);

    return 1;
}

void degree_zero_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 1;
}

void degree_zero_clean(reconstruction_data *d)
{
}
