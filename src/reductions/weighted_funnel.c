#include "weighted_funnel.h"

#include "algorithms.h"

#include <stdlib.h>
#include <assert.h>

typedef struct
{
    node_id *NA;
    node_id *U, *V;
} weighted_funnel_data;

int weighted_funnel_apply(graph *g, node_id u, node_id v, node_weight *offset,
                          buffers *b, change_list *c, reconstruction_data *d)
{
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id w = g->V[u][i];
        if (w == u || w == v)
            continue;

        // check if neighborhood is a clique via is_subset
        if (!set_is_subset_except_two(g->V[u], g->D[u], g->V[w], g->D[w], w, v))
            return 0;
    }

    // Found a uv-funnel
    int *_N = b->fast_sets[0];
    int t = b->t;
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id w = g->V[u][i];
        if (w == u || w == v)
            continue;

        if (g->W[w] + g->W[v] > g->W[u] && !graph_is_neighbor(g, v, w))
            _N[w] = t;
    }

    *offset = g->W[u];
    d->u = u;
    d->v = v;

    weighted_funnel_data *data = malloc(sizeof(weighted_funnel_data));
    data->NA = malloc(sizeof(node_id) * g->D[u]);
    data->U = malloc(sizeof(node_id) * g->D[u] * g->D[v]);
    data->V = malloc(sizeof(node_id) * g->D[u] * g->D[v]);
    d->x = 0;
    d->y = 0;

    if (g->W[u] >= g->W[v])
    {
        d->z = 1;
        graph_deactivate_vertex(g, u);
        for (node_id i = 0; i < g->D[u]; i++)
        {
            node_id w = g->V[u][i];
            if (_N[w] < t)
                graph_deactivate_vertex(g, w);
            else
                data->NA[d->x++] = w;
        }
        for (node_id i = 0; i < g->D[u]; i++)
        {
            node_id w = g->V[u][i];
            if (_N[w] < t)
                continue;

            for (node_id j = 0; j < g->D[v]; j++)
            {
                node_id x = g->V[v][j];
                if (!g->A[x] || graph_is_neighbor(g, w, x))
                    continue;

                graph_insert_edge(g, w, x);
                data->U[d->y] = w;
                data->V[d->y] = x;
                d->y++;
            }
            g->W[w] += g->W[v] - g->W[u];
            reduction_data_queue_distance_one(g, w, c);
        }
    }
    else
    {
        d->z = 2;
        graph_deactivate_vertex(g, u);
        for (node_id i = 0; i < g->D[u]; i++)
        {
            node_id w = g->V[u][i];
            if (w != v && _N[w] < t)
                graph_deactivate_vertex(g, w);
            else if (w != v)
                data->NA[d->x++] = w;
        }
        reduction_data_queue_distance_one(g, v, c);
        g->W[v] -= g->W[u];
        for (node_id i = 0; i < g->D[u]; i++)
        {
            node_id w = g->V[u][i];
            if (_N[w] < t)
                continue;

            for (node_id j = 0; j < g->D[v]; j++)
            {
                node_id x = g->V[v][j];
                if (graph_is_neighbor(g, w, x))
                    continue;

                graph_insert_edge(g, w, x);
                data->U[d->y] = w;
                data->V[d->y] = x;
                d->y++;
            }
            reduction_data_queue_distance_one(g, w, c);
        }
    }

    data->NA = realloc(data->NA, sizeof(node_id) * d->x);
    data->U = realloc(data->U, sizeof(node_id) * d->y);
    data->V = realloc(data->V, sizeof(node_id) * d->y);
    d->data = (void *)data;

    return 1;
}

int weighted_funnel_reduce_graph(graph *g, node_id u, node_weight *offset,
                                 buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > MAX_SIMPLICIAL_VERTEX)
        return 0;

    node_id fv = -1, fv2 = -1;

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        if (g->D[v] < g->D[u] || g->W[v] > g->W[u])
        {
            if (fv >= 0)
                return 0;

            fv = v;
        }
    }

    if (fv >= 0)
    {
        return weighted_funnel_apply(g, u, fv, offset, b, c, d);
    }

    for (node_id i = 0; i < g->D[u] && fv < 0; i++)
    {
        node_id v = g->V[u][i];

        node_id x = 0, y = 0;
        while (x < g->D[u] && y < g->D[v])
        {
            if (g->V[u][x] == g->V[v][y])
            {
                x++;
                y++;
            }
            else if (g->V[u][x] > g->V[v][y])
            {
                y++;
            }
            else if (g->V[u][x] == v)
            {
                x++;
            }
            else
            {
                // Only two candidates, v, and g->V[u][x]
                fv = v;
                fv2 = g->V[u][x];

                break;
            }
        }

        if (fv < 0 && x < g->D[u] && g->V[u][x] == v)
            x++;

        if (fv < 0 && x < g->D[u])
        {
            // Only two candidates, v, and g->V[u][x]
            fv = v;
            fv2 = g->V[u][x];
        }
    }

    if (fv >= 0 && fv2 >= 0)
    {
        return weighted_funnel_apply(g, u, fv, offset, b, c, d) ||
               weighted_funnel_apply(g, u, fv2, offset, b, c, d);
    }

    // TODO if this is reached there is normal reduction

    return 0;
}

void weighted_funnel_restore_graph(graph *g, reconstruction_data *d)
{
    weighted_funnel_data *data = (weighted_funnel_data *)d->data;

    for (node_id i = 0; i < d->y; i++)
    {
        graph_remove_edge(g, data->U[i], data->V[i]);
    }

    if (d->z == 2)
        g->W[d->v] += g->W[d->u];
    for (node_id i = g->D[d->u] - 1; i >= 0; i--)
    {
        node_id v = g->V[d->u][i];
        if (!g->A[v])
            graph_activate_vertex(g, v);
        else if (d->z == 1)
            g->W[v] -= g->W[d->v] - g->W[d->u];
    }
    graph_activate_vertex(g, d->u);
}

void weighted_funnel_reconstruct_solution(int *I, reconstruction_data *d)
{
    weighted_funnel_data *data = (weighted_funnel_data *)d->data;

    int any = 0;
    for (node_id i = 0; i < d->x; i++)
    {
        if (I[data->NA[i]])
            any = 1;
    }
    if (d->z == 1)
    {
        if (!any)
            I[d->u] = 1;
        else
            I[d->v] = 1;
    }
    else if (d->z == 2)
    {
        if (!any && !I[d->v])
            I[d->u] = 1;
    }
}

void weighted_funnel_clean(reconstruction_data *d)
{
    weighted_funnel_data *data = (weighted_funnel_data *)d->data;

    free(data->NA);
    free(data->U);
    free(data->V);

    free(data);
}