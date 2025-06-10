#include "densify.h"

#include "algorithms.h"

#include <assert.h>

int densify_reduce_graph(graph *g, node_id u, node_weight *offset,
                         buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > 128)
        return 0;

    node_id n = 0;
    node_id *diff = b->buffers[0];

    int *fs = b->fast_sets[0];

    d->n = g->l;

    int added_edges = 0;
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        if (g->W[v] < g->W[u])
            continue;

        node_id n = set_minus_except_one(g->V[v], g->D[v], g->V[u], g->D[u], diff, u);

        for (node_id j = 0; j < n; j++)
        {
            node_id x = diff[j];
            for (node_id k = 0; k < g->D[x]; k++)
            {
                node_id y = g->V[x][k];
                if (y == v || y == u)
                    continue;

                if (fs[y] <= b->t)
                    fs[y] = b->t + 1;
                else
                    fs[y]++;

                if (fs[y] - b->t == n && !graph_is_neighbor(g, u, y))
                {
                    graph_add_edge(g, u, y);
                    added_edges++;
                }
            }
        }
        b->t += n;
    }

    if (added_edges > 0)
    {
        // printf("Added %d edges\n", added_edges);
        reduction_data_queue_distance_one(g, u, c);
        return 1;
    }

    return 0;
}

void densify_restore_graph(graph *g, reconstruction_data *d)
{
    graph_undo_changes(g, d->n);
}

void densify_reconstruct_solution(int *I, reconstruction_data *d)
{
}

void densify_clean(reconstruction_data *d)
{
}