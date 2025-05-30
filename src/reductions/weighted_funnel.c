#include "weighted_funnel.h"

#include "algorithms.h"

#include <stdlib.h>
#include <assert.h>

int weighted_funnel_apply(graph *g, node_id u, node_id v, node_weight *offset,
                          buffers *b, change_list *c, reconstruction_data *d)
{
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id w = g->V[u][i];
        if (w == v)
            continue;

        // check if neighborhood is a clique via is_subset
        if (!set_is_subset_except_two(g->V[u], g->D[u], g->V[w], g->D[w], w, v))
            return 0;
    }

    // Found a uv-funnel
    *offset = g->W[u];
    d->u = u;
    d->v = v;
    d->n = g->l;

    node_id *V = malloc(sizeof(node_id) * g->D[u]);
    d->x = 0;

    graph_deactivate_vertex(g, u);
    reduction_data_queue_distance_one(g, u, c);

    if (g->W[u] >= g->W[v])
    {
        d->z = 1;
        for (node_id i = 0; i < g->D[u]; i++)
        {
            node_id w = g->V[u][i];
            if (w == v)
                continue;

            if (g->W[w] + g->W[v] <= g->W[u] || graph_is_neighbor(g, v, w))
            {
                graph_deactivate_vertex(g, w);
            }
            else
            {
                V[d->x++] = w;
                graph_change_vertex_weight(g, w, g->W[w] - (g->W[u] - g->W[v]));
                for (node_id j = 0; j < g->D[v]; j++)
                {
                    node_id x = g->V[v][j];
                    graph_add_edge(g, w, x);
                }
            }
            reduction_data_queue_distance_one(g, w, c);
        }
        graph_deactivate_vertex(g, v);
    }
    else
    {
        d->z = 2;

        graph_change_vertex_weight(g, v, g->W[v] - g->W[u]);
        V[d->x++] = v;

        for (node_id i = 0; i < g->D[u]; i++)
        {
            node_id w = g->V[u][i];
            if (w == v)
                continue;

            if (graph_is_neighbor(g, w, v))
            {
                graph_deactivate_vertex(g, w);
            }
            else
            {
                V[d->x++] = w;
                for (node_id j = 0; j < g->D[v]; j++)
                {
                    node_id x = g->V[v][j];
                    graph_add_edge(g, w, x);
                }
            }
            reduction_data_queue_distance_one(g, w, c);
        }
    }

    d->data = (void *)V;

    return 1;
}

int weighted_funnel_reduce_graph(graph *g, node_id u, node_weight *offset,
                                 buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > MAX_SIMPLICIAL_VERTEX)
        return 0;

    node_id fv1 = -1, fv2 = -1;

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        if (g->D[v] < g->D[u] || g->W[v] > g->W[u])
        {
            if (fv1 >= 0)
                return 0;

            fv1 = v;
        }
    }

    if (fv1 >= 0)
    {
        return weighted_funnel_apply(g, u, fv1, offset, b, c, d);
    }

    for (node_id i = 0; i < g->D[u] && fv1 < 0; i++)
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
                fv1 = v;
                fv2 = g->V[u][x];

                break;
            }
        }

        if (fv1 < 0 && x < g->D[u] && g->V[u][x] == v)
            x++;

        if (fv1 < 0 && x < g->D[u])
        {
            // Only two candidates, v, and g->V[u][x]
            fv1 = v;
            fv2 = g->V[u][x];
        }
    }

    if (fv1 >= 0 && fv2 >= 0)
    {
        return weighted_funnel_apply(g, u, fv1, offset, b, c, d) ||
               weighted_funnel_apply(g, u, fv2, offset, b, c, d);
    }

    // TODO if this is reached there is normal reduction

    return 0;
}

void weighted_funnel_restore_graph(graph *g, reconstruction_data *d)
{
    graph_undo_changes(g, d->n);
}

void weighted_funnel_reconstruct_solution(int *I, reconstruction_data *d)
{
    node_id *V = (node_id *)d->data;

    int any = 0;
    for (node_id i = 0; i < d->x; i++)
    {
        if (I[V[i]])
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
    free(d->data);
}