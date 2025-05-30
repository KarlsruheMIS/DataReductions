#include "v_shape.h"
#include "algorithms.h"

#include <assert.h>
#include <stdlib.h>

int v_shape_reduce_graph(graph *g, node_id u, node_weight *offset,
                         buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] != 2)
        return 0;

    // get neighbors such that w(x) <= w(y)
    if (g->W[g->V[u][0]] > g->W[g->V[u][1]])
    {
        d->y = g->V[u][0];
        d->x = g->V[u][1];
    }
    else
    {
        d->x = g->V[u][0];
        d->y = g->V[u][1];
    }

    // check if v_shape
    if (graph_is_neighbor(g, d->x, d->y))
        return 0;

    // flag to track the reduction case
    d->z = 0;
    d->u = u;
    d->n = g->l;
    *offset = g->W[u];

    if (g->W[u] < g->W[d->x])
    {
        d->z = 1;

        // go through neighborhoods seperately, since edge insertions make it not possible to go through g->V[u]
        for (node_id i = 0; i < g->D[d->x]; i++)
        {
            node_id w = g->V[d->x][i];
            graph_add_edge(g, u, w);
        }
        for (node_id i = 0; i < g->D[d->y]; i++)
        {
            node_id w = g->V[d->y][i];
            graph_add_edge(g, u, w);
        }

        reduction_data_queue_distance_one(g, u, c);

        graph_remove_edge(g, u, d->x);
        graph_remove_edge(g, u, d->y);

        graph_change_vertex_weight(g, d->x, g->W[d->x] - g->W[u]);
        graph_change_vertex_weight(g, d->y, g->W[d->y] - g->W[u]);
    }
    else if (g->W[u] < g->W[d->y])
    {
        d->z = 2;

        graph_deactivate_vertex(g, u);

        graph_change_vertex_weight(g, d->y, g->W[d->y] - g->W[u]);

        for (node_id i = 0; i < g->D[d->y]; i++)
        {
            node_id w = g->V[d->y][i];
            graph_add_edge(g, d->x, w);
        }

        reduction_data_queue_distance_one(g, d->x, c);
        reduction_data_queue_distance_one(g, d->y, c);
    }
    else
    {
        if (g->W[u] >= g->W[d->x] + g->W[d->y])
        {
            d->z = 3;

            graph_deactivate_neighborhood(g, u);

            reduction_data_queue_distance_two(g, u, c);
        }
        else
        {
            d->z = 4;

            d->v = d->x;
            for (node_id i = 0; i < g->D[d->y]; i++)
            {
                node_id v = g->V[d->y][i];
                graph_add_edge(g, d->v, v);
            }

            graph_change_vertex_weight(g, d->v, (g->W[d->x] + g->W[d->y]) - g->W[u]);
            graph_deactivate_vertex(g, u);
            graph_deactivate_vertex(g, d->y);

            reduction_data_queue_distance_one(g, d->v, c);
        }
    }

    return 1;
}

void v_shape_restore_graph(graph *g, reconstruction_data *d)
{
    graph_undo_changes(g, d->n);
}

void v_shape_reconstruct_solution(int *I, reconstruction_data *d)
{
    if (d->z == 1)
    {
        if (!I[d->x] && !I[d->y])
            I[d->u] = 1;
        else if (I[d->u])
            I[d->u] = 0;
    }
    else if (d->z == 2)
    {
        if (!I[d->x] && !I[d->y])
            I[d->u] = 1;
    }
    else if (d->z == 3)
    {
        I[d->u] = 1;
    }
    else
    {
        if (I[d->v])
        {
            I[d->v] = 0;
            I[d->x] = 1;
            I[d->y] = 1;
        }
        else
        {
            I[d->u] = 1;
        }
    }
}

void v_shape_clean(reconstruction_data *d)
{
}
