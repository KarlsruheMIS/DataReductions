#include "simplicial_vertex.h"
#include "algorithms.h"

#include <assert.h>

int simplicial_vertex_reduce_graph(graph *g, node_id u, node_weight *offset,
                                   buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > MAX_SIMPLICIAL_VERTEX)
        return 0;

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        // if d[v] is smaller than d[u], then N(u) can not be clique
        // if v and u have same degree, both are potentially simplicial then, we want to check only for the highest weight vertex
        if (g->D[v] < g->D[u] || (g->D[v] == g->D[u] && g->W[v] > g->W[u]))
            return 0;

        b->fast_sets[0][v] = b->t;
    }
    b->fast_sets[0][u] = b->t;

    // check if neighborhood is a clique
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        for (node_id j = 0; j < g->D[v]; j++)
        {
            node_id w = g->V[v][j];

            if (b->fast_sets[0][w] != b->t)
                return 0;
        }
    }

    *offset = g->W[u];
    d->u = u;
    graph_deactivate_neighborhood(g, u);

    reduction_data_queue_distance_two(g, u, c);

    return 1;
}

void simplicial_vertex_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_activate_neighborhood(g, d->u);
}

void simplicial_vertex_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 1;
}

void simplicial_vertex_clean(reconstruction_data *d)
{
}