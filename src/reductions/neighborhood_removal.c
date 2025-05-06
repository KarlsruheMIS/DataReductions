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
    graph_deactivate_neighborhood(g, u);
    d->u = u;

    c->n = 0;
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        for (node_id j = 0; j < g->D[v]; j++)
        {
            node_id w = g->V[v][j];

            if (g->A[w])
                c->V[c->n++] = w;

            if (c->n == c->_a)
                return 1;
        }
    }

    return 1;
}

void neighborhood_removal_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_activate_neighborhood(g, d->u);
}

void neighborhood_removal_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 1;
}

void neighborhood_removal_clean(reconstruction_data *d)
{
}
