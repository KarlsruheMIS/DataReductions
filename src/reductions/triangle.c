#include "triangle.h"
#include "algorithms.h"

#include <assert.h>

int triangle_reduce_graph(graph *g, node_id u, node_weight *offset,
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

    // check if triangle
    if (!graph_is_neighbor(g, d->x, d->y))
        return 0;

    // flag to track the reduction case
    d->z = 0;

    d->u = u;
    d->n = g->l;
    *offset = g->W[u];

    if (g->W[u] < g->W[d->x])
    {
        d->z = 1;

        graph_deactivate_vertex(g, u);

        graph_change_vertex_weight(g, d->x, g->W[d->x] - g->W[u]);
        graph_change_vertex_weight(g, d->y, g->W[d->y] - g->W[u]);

        reduction_data_queue_distance_two(g, u, c);
    }
    else if (g->W[u] < g->W[d->y])
    {
        d->z = 2;

        graph_deactivate_vertex(g, u);
        graph_deactivate_vertex(g, d->x);

        graph_change_vertex_weight(g, d->y, g->W[d->y] - g->W[u]);

        reduction_data_queue_distance_one(g, d->x, c);
        reduction_data_queue_distance_one(g, d->y, c);
    }
    else
    {
        d->z = 3;

        graph_deactivate_neighborhood(g, u);

        reduction_data_queue_distance_two(g, u, c);
    }

    return 1;
}

void triangle_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_undo_changes(g, d->n);
}

void triangle_reconstruct_solution(int *I, reconstruction_data *d)
{
    if (d->z == 1)
    {
        if (I[d->x] == 0 && I[d->y] == 0)
            I[d->u] = 1;
    }
    else if (d->z == 2)
    {
        if (I[d->y] == 0)
            I[d->u] = 1;
    }
    else
    {
        I[d->u] = 1;
    }
}

void triangle_clean(reconstruction_data *d)
{
}
