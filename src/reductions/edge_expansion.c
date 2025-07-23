#include "unconfined.h"
#include "algorithms.h"

#include <assert.h>

int edge_expansion_reduce_graph(graph *g, node_id u, node_weight *offset,
                                buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    assert(N_BUFFERS >= 1);

    int *NS_B = b->fast_sets[0];
    int *NS2 = b->buffers[0];
    int t = b->t;

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        NS_B[v] = t;
    }

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        if (g->W[v] < g->W[u])
            continue;

        node_weight nw = 0;
        int n = 0;

        for (node_id j = 0; j < g->D[v]; j++)
        {
            node_id w = g->V[v][j];
            if (NS_B[w] == t)
                continue;

            nw += g->W[w];
            NS2[n++] = w;
        }

        if (g->W[u] + nw <= g->W[v]) // Exclude u
        {
            *offset = 0;
            d->x = 0;
            d->u = u;
            d->n = g->l;

            graph_deactivate_vertex(g, u);

            reduction_data_queue_distance_one(g, u, c);

            return 1;
        }
        else if (g->W[u] + nw > g->W[v] && n == 1) // Edge expansion
        {
            node_id x = NS2[0];

            *offset = 0;
            d->x = 1;
            d->u = u;
            d->v = x;
            d->n = g->l;

            graph_change_vertex_weight(g, u, g->W[u] + g->W[x]);

            graph_add_edge(g, u, x);

            for (node_id j = 0; j < g->D[x]; j++)
            {
                node_id w = g->V[x][j];
                if (NS_B[w] == t)
                    continue;

                graph_add_edge(g, u, w);
            }

            reduction_data_queue_distance_one(g, u, c);

            return 1;
        }
    }

    return 0;
}

void edge_expansion_restore_graph(graph *g, reconstruction_data *d)
{
    assert(d->x || !g->A[d->u]);

    graph_undo_changes(g, d->n);
}

void edge_expansion_reconstruct_solution(int *I, reconstruction_data *d)
{
    if (d->x == 1 && I[d->u])
        I[d->v] = 1;
}

void edge_expansion_clean(reconstruction_data *d)
{
}
