#include "unconfined.h"
#include "algorithms.h"

#include <assert.h>

int unconfined_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    int n = 0, m = 0, t = b->t;

    assert(N_BUFFERS >= 3);

    node_id *S = b->buffers[0], *NS = b->buffers[1];
    int *S_B = b->fast_sets[0], *NS_B = b->fast_sets[1], *NIS_B = b->fast_sets[2];

    S[n++] = u;
    S_B[u] = t;
    NIS_B[u] = t;
    for (node_id i = 0; i < g->D[u] && m <= MAX_UNCONFINED; i++)
    {
        node_id v = g->V[u][i];

        NS[m++] = v;
        NS_B[v] = t;
        NIS_B[v] = t;
    }

    int res = 0, first = 1;
    while (m > 0 && m <= MAX_UNCONFINED)
    {
        node_id v = NS[--m];
        NS_B[v] = t - 1;
        if (S_B[v] == t)
            continue;

        long long sw = 0, dw = 0;
        node_id dn = 0, x = -1;

        if (first && g->W[v] < g->W[u]) // Not a child
            continue;

        for (node_id i = 0; i < g->D[v]; i++)
        {
            node_id w = g->V[v][i];

            if (S_B[w] == t)
                sw += g->W[w];
            else if (NIS_B[w] < t)
            {
                dw += g->W[w];
                dn++;
                x = w;
            }
        }

        if (g->W[v] < sw) // Not a chld
            continue;

        if (sw + dw <= g->W[v]) // Condition 1. can reduce u
        {
            res = 1;
            break;
        }
        else if (sw + dw > g->W[v] && dn == 1) // Extending child
        {
            first = 0;
            S[n++] = x;
            S_B[x] = t;
            NIS_B[x] = t;
            for (node_id i = 0; i < g->D[x]; i++)
            {
                int w = g->V[x][i];
                if (NS_B[w] == t)
                    continue;
                NS[m++] = w;
                NS_B[w] = t;
                NIS_B[w] = t;
            }
        }
    }

    if (res)
    {
        *offset = 0;
        d->u = u;
        graph_deactivate_vertex(g, u);

        reduction_data_queue_distance_one(g, u, c);

        return 1;
    }
    return 0;
}

void unconfined_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_activate_vertex(g, d->u);
}

void unconfined_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 0;
}

void unconfined_clean(reconstruction_data *d)
{
}