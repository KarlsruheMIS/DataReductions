#include "extended_struction.h"
#include "algorithms.h"

#include <assert.h>
#include <stdlib.h>

int STRUCTION_MAX_DEGREE = 8;
int STRUCTION_MAX_NODES = 16;

node_id struction_enumerate_independent_sets(graph *g, node_id u, buffers *b)
{
    assert(N_BUFFERS >= 4);

    node_id *V = b->buffers[0];
    node_id *E = b->buffers[1];

    node_id *IS = b->buffers[2];
    node_id *Branch = b->buffers[3];

    node_weight *W = b->buffers_weigth[0];
    node_weight cost = 0;

    int *Ni = b->fast_sets[0];
    int t = b->t;

    node_id p = 0, n = 0, n_is = 0, m = 0;
    V[0] = 0;
    while (p >= 0)
    {
        // Scan forward for next to add
        while (p < g->D[u])
        {
            node_id v = g->V[u][p];

            if (Ni[v] < t)
                break;

            Branch[p] = 0;
            p++;
        }

        if (p == g->D[u])
        {
            if (cost > g->W[u]) // Add IS
            {
                if (n_is >= STRUCTION_MAX_NODES)
                    return -1;

                W[n_is] = cost;
                m += n;
                for (int i = 0; i < n; i++)
                    E[V[n_is] + i] = IS[i];

                V[++n_is] = m;
            }
            // Return to last branch and exclude
            p--;
            while (p >= 0 && Branch[p] == 0)
                p--;

            if (p >= 0)
            {
                node_id v = g->V[u][p];
                Branch[p] = 0;
                n--;
                cost -= g->W[v];
                for (int i = 0; i < g->D[v]; i++)
                    Ni[g->V[v][i]]--;
                p++;
            }
        }
        else
        {
            node_id v = g->V[u][p];
            Branch[p] = 1;
            IS[n++] = v;
            cost += g->W[v];
            for (int i = 0; i < g->D[v]; i++)
            {
                node_id w = g->V[v][i];
                if (Ni[w] < t)
                    Ni[w] = t;
                else
                    Ni[w]++;
            }
            p++;
        }
    }

    return n_is;
}

int struction_reduce_graph(graph *g, node_id u, node_weight *offset,
                           buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > STRUCTION_MAX_DEGREE || g->D[u] < 3)
        return 0;

    int n = struction_enumerate_independent_sets(g, u, b);
    b->t += g->D[u];

    if (n < 0)
        return 0;

    node_id *V = b->buffers[0];
    node_id *E = b->buffers[1];

    node_weight *W = b->buffers_weigth[0];

    // Reduce graph
    *offset = g->W[u];
    d->n = g->l;
    d->u = u;
    d->x = n;

    node_id *data = malloc(sizeof(node_id) * (n + (n + 1) + V[n]));
    node_id *ID = data, *_V = data + n, *_E = data + n + (n + 1);

    d->data = (void *)data;

    graph_deactivate_neighborhood(g, u);

    for (node_id i = 0; i < n; i++)
    {
        ID[i] = g->n;
        _V[i] = V[i];

        graph_add_vertex(g, W[i] - g->W[u]);

        for (node_id j = V[i]; j < V[i + 1]; j++)
        {
            node_id v = E[j];
            _E[j] = v;

            for (node_id k = 0; k < g->D[v]; k++)
            {
                node_id w = g->V[v][k];
                if (g->A[w])
                    graph_add_edge(g, ID[i], w);
            }
        }
    }
    _V[n] = V[n];

    // Queue changed
    for (node_id i = 0; i < n; i++)
    {
        reduction_data_queue_distance_one(g, ID[i], c);
    }

    // Make clique
    for (node_id i = 0; i < n; i++)
    {
        for (node_id j = i + 1; j < n; j++)
        {
            graph_add_edge(g, ID[i], ID[j]);
        }
    }

    return 1;
}

void struction_restore_graph(graph *g, reconstruction_data *d)
{
    graph_undo_changes(g, d->n);
}

void struction_reconstruct_solution(int *I, reconstruction_data *d)
{
    node_id n = d->x;
    node_id *data = (node_id *)d->data;
    node_id *ID = data, *V = data + n, *E = data + n + (n + 1);

    int any = 0;
    for (node_id i = 0; i < n; i++)
    {
        node_id u = ID[i];
        if (I[u])
        {
            assert(!any);
            any = 1;
            I[u] = 0;
            for (node_id j = V[i]; j < V[i + 1]; j++)
            {
                node_id v = E[j];
                I[v] = 1;
            }
        }
    }
    if (!any)
        I[d->u] = 1;
}

void struction_clean(reconstruction_data *d)
{
    node_id *data = (node_id *)d->data;
    free(data);
}