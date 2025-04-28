
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
    node_id pos = lower_bound(g->V[d->x], g->D[d->x], d->y);
    if (pos < g->D[d->x] && g->V[d->x][pos] == d->y)
        return 0;

    // flag to track the reduction case
    d->z = 0;
    assert(!graph_is_neighbor(g, d->x, d->y));

    d->u = u;
    *offset = g->W[u];

    if (g->W[u] < g->W[d->x])
    {
        d->z = 1;

        node_id *added_edges = malloc(sizeof(node_id) * (g->D[d->x] + g->D[d->y]));
        d->n = 0;

        c->n = 3;
        c->V[0] = u;
        c->V[1] = d->x;
        c->V[2] = d->y;

        b->fast_sets[0][u] = b->t;
        // go through neighborhoods seperately, since edge insertions make it not possible to go through g->V[u]
        for (node_id i = 0; i < g->D[d->x]; i++)
        {
            node_id w = g->V[d->x][i];
            if (b->fast_sets[0][w] < b->t)
            {
                b->fast_sets[0][w] = b->t;
                graph_insert_edge(g, u, w);
                added_edges[d->n++] = w;
                c->V[c->n++] = w;
            }
        }
        for (node_id i = 0; i < g->D[d->y]; i++)
        {
            node_id w = g->V[d->y][i];
            if (b->fast_sets[0][w] < b->t)
            {
                b->fast_sets[0][w] = b->t;
                graph_insert_edge(g, u, w);
                added_edges[d->n++] = w;
                c->V[c->n++] = w;
            }
        }
        d->data = (void *)added_edges; 

        graph_remove_edge(g, u, d->x);
        graph_remove_edge(g, u, d->y);

        g->W[d->x] -= g->W[u];
        g->W[d->y] -= g->W[u];
    }
    else if (g->W[u] < g->W[d->y])
    {
        d->z = 2;

        node_id *added_edges = malloc(sizeof(node_id) * (g->D[d->x] + g->D[d->y]));
        d->n = 0;

        graph_deactivate_vertex(g, u);

        g->W[d->y] -= g->W[u];

        c->n = 2;
        c->V[1] = d->x;
        c->V[0] = d->y;

        for (node_id i = 0; i < g->D[d->x]; i++)
        {
            node_id w = g->V[d->x][i];
            b->buffers[0][w] = b->t;
        }
        for (node_id i = 0; i < g->D[d->y]; i++)
        {
            node_id w = g->V[d->y][i];

            if (b->buffers[0][w] < b->t)
            {
                b->buffers[0][w] = b->t;
                graph_insert_edge(g, d->x, w);
                added_edges[d->n++] = w;
                c->V[c->n++] = w;
            }
        }
        d->data = (void *)added_edges; 
    }
    else
    {
        if (g->W[u] >= g->W[d->x] + g->W[d->y])
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
                }
            }
            return 1;
        }
        else
        {
            d->z = 4;

            d->v = g->n; // node_id of new created vertex for fold

            graph_add_vertex(g, g->W[d->x] + g->W[d->y] - g->W[u]);

            c->n = 1;
            c->V[0] = d->v;
            b->fast_sets[0][u] = b->t;

            for (node_id i = 0; i < g->D[u]; i++)
            {
                node_id v = g->V[u][i];

                for (node_id j = 0; j < g->D[v]; j++)
                {
                    node_id w = g->V[v][j];

                    if (b->fast_sets[0][w] < b->t)
                    {
                        b->fast_sets[0][w] = b->t;
                        graph_insert_edge(g, d->v, w);
                        c->V[c->n++] = w;
                    }
                }
            }
            graph_deactivate_neighborhood(g, u);
        }
    }


    return 1;
}

void v_shape_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    if (d->z == 1)
    {
        node_id * added_edges = (node_id *)d->data;
        for (node_id i = 0; i < d->n; i++)
        {
            node_id v = added_edges[i];
            graph_remove_edge(g, d->u, v);
        }

        graph_insert_edge(g, d->u, d->x);
        graph_insert_edge(g, d->u, d->y);

        g->W[d->x] += g->W[d->u];
        g->W[d->y] += g->W[d->u];
    }
    else if (d->z == 2)
    {
        graph_activate_vertex(g, d->u);

        node_id * added_edges = (node_id *)d->data;
        for (node_id i = 0; i < d->n; i++)
        {
            node_id v = added_edges[i];
            graph_remove_edge(g, d->u, v);
        }

        g->W[d->y] += g->W[d->u];
    }
    else if (d->z == 3)
    {
        graph_activate_neighborhood(g, d->u);
    }
    else
    {
        graph_deactivate_vertex(g, d->v);
        graph_remove_last_added_vertex(g);

        graph_activate_neighborhood(g, d->u);
    }
}

void v_shape_reconstruct_solution(int *I, reconstruction_data *d)
{
    if (d->z == 1)
    {
        if (I[d->x] == 0 && I[d->y] == 0)
            I[d->u] = 1;
    }
    else if (d->z == 2)
    {
        if (I[d->x] == 0 && I[d->y] == 0)
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
    node_id * added_edges = (node_id *)d->data;
    free(added_edges);
}
