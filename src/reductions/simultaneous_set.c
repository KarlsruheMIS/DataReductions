#include "simultaneous_set.h"
#include "algorithms.h"

#include <assert.h>
#include <stdlib.h>

long long get_total_weight(graph *g, node_id u, int *fs, int k)
{
    long long res = 0;
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        if (fs[v] < k)
            res += g->W[v];
    }
    return res;
}

int simultaneous_set_test(graph *g, node_id u, int *fs, int k)
{
    return get_total_weight(g, u, fs, k) <= g->W[u];
}

int simultaneous_set_test_reverse(graph *g, node_id u, node_id v, int *fs, int k)
{
    for (node_id i = 0; i < g->D[v]; i++)
    {
        node_id w = g->V[v][i];
        fs[w] = k;
    }

    int res = get_total_weight(g, u, fs, k) <= g->W[u];

    for (node_id i = 0; i < g->D[v]; i++)
    {
        node_id w = g->V[v][i];
        fs[w] = k - 1;
    }

    return res;
}

int simultaneous_set_reduce_graph(graph *g, node_id u, node_weight *offset,
                                  buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u] && N_BUFFERS >= 3);

    int *fs1 = b->fast_sets[0], *fs2 = b->fast_sets[1], *fs3 = b->fast_sets[2];
    int k = b->t;

    // Assume vertex is in the solution
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        fs1[v] = k;
    }

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        for (node_id j = 0; j < g->D[v]; j++)
        {
            node_id w = g->V[v][j];
            if (w == u || fs1[w] == k)
                continue;

            fs2[w] = k;
            if (simultaneous_set_test(g, w, fs1, k) &&
                simultaneous_set_test_reverse(g, u, w, fs3, k))
            {
                *offset = 0;

                d->u = u;
                d->v = w;

                graph_deactivate_vertex(g, w);
                g->W[u] += g->W[w];

                d->n = 0;
                node_id *added_edges = malloc(sizeof(node_id) * g->D[w]);
                for (node_id l = 0; l < g->D[w]; l++)
                {
                    node_id x = g->V[w][l];
                    if (fs3[x] < k && !graph_is_neighbor(g, u, x))
                    {
                        fs3[x] = k;
                        added_edges[d->n++] = x;
                        graph_insert_edge(g, u, x);
                    }
                }
                d->data = (void *)added_edges;

                reduction_data_queue_distance_one(g, u, c);

                return 1;
            }
        }
    }

    return 0;
}

void simultaneous_set_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->v]);

    node_id *added_edges = (node_id *)d->data;
    for (node_id i = 0; i < d->n; i++)
    {
        graph_remove_edge(g, d->u, added_edges[i]);
    }

    graph_activate_vertex(g, d->v);

    g->W[d->u] -= g->W[d->v];
}

void simultaneous_set_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->v] = I[d->u];
}

void simultaneous_set_clean(reconstruction_data *d)
{
    node_id *added_edges = (node_id *)d->data;
    free(added_edges);
}