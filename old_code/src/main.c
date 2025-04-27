#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graph.h"
#include "reductions.h"

long long mwis_validate(graph *g, int *independent_set)
{
    long long cost = 0;
    for (int u = 0; u < g->N; u++)
    {
        if (!independent_set[u])
            continue;

        cost += g->W[u];
        for (int i = g->V[u]; i < g->V[u + 1]; i++)
        {
            int v = g->E[i];
            if (independent_set[v])
                return -1;
        }
    }
    return cost;
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph *g = graph_parse(f);
    fclose(f);

    printf("%d %d\n", g->N, g->V[g->N] / 2);

    long long offset = 0;
    int *A = malloc(sizeof(int) * g->N);
    int *S = malloc(sizeof(int) * g->N);
    int *reverse_mapping = malloc(sizeof(int) * g->N);

    for (int i = 0; i < g->N; i++)
    {
        A[i] = 1;
        S[i] = 0;
    }

    kernelize_csr(g->N, g->V, g->E, g->W, A, S, &offset, 3600, 2,
                  reduction_neighborhood_csr,
                  reduction_boolean_width_csr,
                  reduction_domination_csr,
                  reduction_clique_csr,
                  reduction_extended_unconfined_csr);

    int b = -1;
    for (int u = 0; u < g->N; u++)
    {
        if (!A[u])
            continue;

        if (b < 0 || g->W[b] < g->W[u])
            b = u;

        // int d = 0;
        // for (int i = g->V[u]; i < g->V[u + 1]; i++)
        //     if (A[g->E[i]])
        //         d++;

        // if (d < bd)
        // {
        //     b = u;
        //     bd = d;
        // }
    }

    graph_visualize_neighborhood(g, A, b);

    int Nr = 0, Mr = 0;
    for (int u = 0; u < g->N; u++)
    {
        if (!A[u])
            continue;

        Nr++;
        for (int i = g->V[u]; i < g->V[u + 1]; i++)
        {
            int v = g->E[i];
            if (A[v])
                Mr++;
        }
    }

    printf("%s %d %d %lld\n", argv[1], Nr, Mr / 2, offset);

    free(A);
    free(S);
    graph_free(g);

    return 0;
}