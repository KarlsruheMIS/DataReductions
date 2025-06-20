#include "twin.h"
#include "algorithms.h"

#include <assert.h>

int twin_reduce_graph(graph *g, node_id u, node_weight *offset,
                      buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    node_id md = -1;
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        if (md < 0 || g->D[v] < g->D[md])
            md = v;
    }

    if (md < 0 || g->D[md] > MAX_TWIN)
        return 0;

    for (node_id i = 0; i < g->D[md]; i++)
    {
        node_id v = g->V[md][i];

        if (v != u && set_is_equal(g->V[u], g->D[u], g->V[v], g->D[v]))
        {
            d->u = u;
            d->v = v;
            d->n = g->l;

            *offset = 0;

            graph_deactivate_vertex(g, v);

            graph_change_vertex_weight(g, u, g->W[u] + g->W[v]);

            reduction_data_queue_distance_one(g, u, c);

            return 1;
        }
    }

    return 0;
}

void twin_restore_graph(graph *g, reconstruction_data *d)
{
    assert(g->A[d->u] && !g->A[d->v]);

    graph_undo_changes(g, d->n);
}

void twin_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->v] = I[d->u];
}

void twin_clean(reconstruction_data *d)
{
}