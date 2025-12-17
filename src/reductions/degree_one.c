#include "degree_one.h"

#include <assert.h>

int degree_one_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] != 1)
        return 0;

    *offset = g->W[u];
    d->u = u;
    d->v = g->V[u][0];
    d->n = g->l;
    d->z = 0;

    if (g->W[u] >= g->W[d->v])
    {
        graph_deactivate_neighborhood(g, u);
        reduction_data_queue_distance_one(g, d->v, c);
    }
    else
    {
        graph_deactivate_vertex(g, u);
        graph_change_vertex_weight(g, d->v, g->W[d->v] - g->W[u]);
        reduction_data_queue_distance_one(g, d->v, c);
        d->z = 1; // flag for folding
    }

    reduction_data_queue_distance_one(g, d->v, c);

    return 1;
}

void degree_one_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_undo_changes(g, d->n);
}

void degree_one_reconstruct_solution(int *I, reconstruction_data *d)
{
    if (!d->z || !I[d->v])
    {
        I[d->u] = 1;
    }
}

void degree_one_clean(reconstruction_data *d)
{
}
