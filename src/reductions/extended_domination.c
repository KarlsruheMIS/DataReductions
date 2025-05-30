#include "extended_domination.h"
#include "algorithms.h"

#include <assert.h>

int extended_domination_reduce_graph(graph *g, node_id u, node_weight *offset,
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

    if (md < 0 || g->D[md] > MAX_DOMINATION)
        return 0;

    *offset = 0;
    d->n = g->l;

    if (g->W[md] > g->W[u] && g->D[md] >= g->D[u] &&
        set_is_subset_except_one(g->V[u], g->D[u], g->V[md], g->D[md], md))
    {
        d->u = u;
        d->v = md;

        graph_change_vertex_weight(g, md, g->W[md] - g->W[u]);
        graph_remove_edge(g, u, md);

        reduction_data_queue_distance_one(g, u, c);
        reduction_data_queue_distance_one(g, md, c);

        return 1;
    }

    for (node_id i = 0; i < g->D[md]; i++)
    {
        int v = g->V[md][i];

        if (v != u && g->W[v] > g->W[u] && g->D[v] >= g->D[u] && graph_is_neighbor(g, u, v) &&
            set_is_subset_except_one(g->V[u], g->D[u], g->V[v], g->D[v], v))
        {
            d->u = u;
            d->v = v;

            graph_change_vertex_weight(g, v, g->W[v] - g->W[u]);
            graph_remove_edge(g, u, v);

            reduction_data_queue_distance_one(g, u, c);
            reduction_data_queue_distance_one(g, v, c);

            return 1;
        }
    }

    return 0;
}

void extended_domination_restore_graph(graph *g, reconstruction_data *d)
{
    graph_undo_changes(g, d->n);
}

void extended_domination_reconstruct_solution(int *I, reconstruction_data *d)
{
    if (I[d->v])
        I[d->u] = 0;
}

void extended_domination_clean(reconstruction_data *d)
{
}