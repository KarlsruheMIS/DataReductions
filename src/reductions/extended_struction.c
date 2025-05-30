#include "extended_struction.h"
#include "algorithms.h"

#include <assert.h>
#include <stdlib.h>

typedef struct
{
    node_id n, m;
    node_id *V, *E, *ID;
    node_weight *W;
} extended_struction_data;

extended_struction_data *extended_struction_init()
{
    extended_struction_data *es = malloc(sizeof(extended_struction_data));

    es->n = 0;
    es->m = 0;

    es->V = malloc(sizeof(node_id) * (MAX_STRUCTION_NODES + 1));
    es->E = malloc(sizeof(node_id) * MAX_STRUCTION_NODES * MAX_STRUCTION_NODES);
    es->ID = malloc(sizeof(node_id) * MAX_STRUCTION_NODES);
    es->W = malloc(sizeof(node_weight) * MAX_STRUCTION_NODES);

    es->V[0] = 0;

    return es;
}

void extended_struction_free(extended_struction_data *es)
{
    free(es->V);
    free(es->E);
    free(es->ID);
    free(es->W);

    free(es);
}

void extended_struction_enumerate_independent_sets(graph *g, node_id u, extended_struction_data *es, buffers *b)
{
    int *Branch = malloc(sizeof(int) * MAX_STRUCTION_NODES);
    int *IS = malloc(sizeof(int) * MAX_STRUCTION_NODES);

    for (int i = 0; i < MAX_STRUCTION_NODES; i++)
        Branch[i] = 0;

    node_weight W = 0;

    int *Ni = b->fast_sets[0];
    int t = b->t;

    int p = 0, n = 0;
    while (p >= 0 && es->n < MAX_STRUCTION_NODES)
    {
        // Scan forward for next to add
        while (p < g->D[u])
        {
            node_id w = g->V[u][p];
            int valid = 1;
            for (node_id j = 0; j < g->D[w]; j++)
            {
                if (Ni[g->V[w][j]] == t)
                {
                    valid = 0;
                    break;
                }
            }
            if (valid)
                break;

            Branch[p] = 0;
            p++;
        }

        if (p == g->D[u])
        {
            if (W > g->W[u]) // Add IS
            {
                es->W[es->n] = W;
                es->m += n;
                for (int i = 0; i < n; i++)
                    es->E[es->V[es->n] + i] = IS[i];

                es->n++;
                es->V[es->n] = es->m;
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
                W -= g->W[v];
                Ni[v] = t - 1;
                p++;
            }
        }
        else
        {
            node_id v = g->V[u][p];
            Branch[p] = 1;
            IS[n++] = v;
            W += g->W[v];
            Ni[v] = t;
            p++;
        }
    }

    free(Branch);
    free(IS);
}

int extended_struction_reduce_graph(graph *g, node_id u, node_weight *offset,
                                    buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > MAX_STRUCTION_DEGREE)
        return 0;

    extended_struction_data *es = extended_struction_init();

    extended_struction_enumerate_independent_sets(g, u, es, b);

    // Reduce graph
    if (es->n < MAX_STRUCTION_NODES)
    {
        *offset = g->W[u];
        d->n = g->l;

        graph_deactivate_neighborhood(g, u);

        for (node_id i = 0; i < es->n; i++)
        {
            graph_add_vertex(g, es->W[i] - g->W[u]);
            es->ID[i] = g->n - 1;

            for (node_id j = es->V[i]; j < es->V[i + 1]; j++)
            {
                node_id v = es->E[j];
                for (node_id k = 0; k < g->D[v]; k++)
                {
                    node_id w = g->V[v][k];
                    if (g->A[w])
                        graph_add_edge(g, g->n - 1, w);
                }
            }
        }

        // Queue changed
        for (node_id i = 0; i < es->n; i++)
        {
            reduction_data_queue_distance_one(g, es->ID[i], c);
        }

        // Make clique
        for (node_id i = 0; i < es->n; i++)
        {
            for (node_id j = i + 1; j < es->n; j++)
            {
                graph_add_edge(g, es->ID[i], es->ID[j]);
            }
        }

        d->u = u;
        es->E = realloc(es->E, sizeof(node_id) * es->m);
        es->V = realloc(es->V, sizeof(node_id) * (es->n + 1));
        es->ID = realloc(es->ID, sizeof(node_id) * es->n);
        es->W = realloc(es->W, sizeof(node_weight) * es->n);
        d->data = (void *)es;

        return 1;
    }

    extended_struction_free(es);

    return 0;
}

void extended_struction_restore_graph(graph *g, reconstruction_data *d)
{
    graph_undo_changes(g, d->n);
}

void extended_struction_reconstruct_solution(int *I, reconstruction_data *d)
{
    extended_struction_data *es = (extended_struction_data *)d->data;
    int any = 0;
    for (node_id i = 0; i < es->n; i++)
    {
        node_id u = es->ID[i];
        if (I[u])
        {
            assert(!any);
            any = 1;
            I[u] = 0;
            for (node_id j = es->V[i]; j < es->V[i + 1]; j++)
            {
                node_id v = es->E[j];
                I[v] = 1;
            }
        }
    }
    if (!any)
        I[d->u] = 1;
}

void extended_struction_clean(reconstruction_data *d)
{
    extended_struction_data *es = (extended_struction_data *)d->data;
    extended_struction_free(es);
}