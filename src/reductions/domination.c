#include "domination.h"
#include "algorithms.h"

#include <assert.h>

int domination_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    for (node_id i = 0; i < g->D[u]; i++)
    {
        int v = g->V[u][i];

        if (g->W[v] >= g->W[u] && g->D[v] <= g->D[u] &&
            test_subset_except_one(g->V[v], g->D[v], g->V[u], g->D[u], u))
        {
            *offset = 0;
            d->u = u;
            graph_deactivate_vertex(g, u);

            c->n = 0;
            for (node_id j = 0; j < g->D[u]; j++)
                c->V[c->n++] = g->V[u][j];

            return 1;
        }
    }

    return 0;
}

void domination_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_activate_vertex(g, d->u);
}

void domination_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 0;
}

void domination_clean(reconstruction_data *d)
{
}