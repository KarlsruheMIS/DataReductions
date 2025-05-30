#include "neighborhood_removal.h"

#include <assert.h>

int neighborhood_removal_reduce_graph(graph *g, node_id u, node_weight *offset,
                                      buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    long long w_neighborhood = 0;
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id neighbor = g->V[u][i];

        w_neighborhood += g->W[neighbor];
        if (w_neighborhood > g->W[u])
            return 0;
    }

    assert(w_neighborhood <= g->W[u]);

    *offset = g->W[u];
    d->u = u;
    d->n = g->l;

    graph_deactivate_neighborhood(g, u);

    reduction_data_queue_distance_two(g, u, c);

    return 1;
}

void neighborhood_removal_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_undo_changes(g, d->n);
}

void neighborhood_removal_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 1;
}

void neighborhood_removal_clean(reconstruction_data *d)
{
}
