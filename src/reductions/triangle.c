
#include "triangle.h"
#include "algorithms.h"

#include <assert.h>

int triangle_reduce_graph(graph *g, node_id u, node_weight *offset, buffers *b, change_list *c, reconstruction_data *d)
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
    if (lower_bound(g->V[d->x], g->D[d->x], d->y) == g->D[d->x])
        return 0;

    // flag to track the reduction case
    d->z = 0; 

    d->u = u;
    *offset = g->W[u];

    if (g->W[u] < g->W[d->x])
    {
        d->z = 1;

        graph_deactivate_vertex(g, u);

        g->W[d->x] -= g->W[u];
        g->W[d->y] -= g->W[u];

        c->n = 2;
        c->V[0] = d->x;
        c->V[1] = d->y;
    }
    else if (g->W[u] < g->W[d->y])
    {
        d->z = 2;

        graph_deactivate_vertex(g, u);
        graph_deactivate_vertex(g, d->x);

        g->W[d->y] -= g->W[u];

        c->n = 1;
        c->V[0] = d->y;
        for (node_id i = 0; i < g->D[d->x]; i++)
        {
            node_id w = g->V[d->x][i];

            if (g->A[w])
                c->V[c->n++] = w;

            if (c->n == c->_a)
                return 1;
        }
    }
    else
    {
        d->z = 3;

        graph_deactivate_neighborhood(g, u);

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
    }

    return 1;
}

void triangle_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    if (d->z == 1)
    {
        graph_activate_vertex(g, d->u);

        g->W[d->x] += g->W[d->u];
        g->W[d->y] += g->W[d->u];
    }
    else if (d->z == 2)
    {
        graph_activate_vertex(g, d->u);
        graph_activate_vertex(g, d->x);

        g->W[d->y] += g->W[d->u];
    }
    else
    {
        graph_activate_neighborhood(g, d->u);
    }
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
