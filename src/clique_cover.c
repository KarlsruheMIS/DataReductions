#define _GNU_SOURCE

#include "clique_cover.h"

#include <stdlib.h>

void clique_cover_first_fit(graph *g, int *C, int *P, int *T, node_id *O)
{
    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            P[u] = -1;
        else
            P[u] = g->n - u;
    }

    clique_cover_from_priority(g, C, P, T, O);
}

void clique_cover_largest_degree_first(graph *g, int *C, int *P, int *T, node_id *O)
{
    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            P[u] = -1;
        else
            P[u] = g->D[u];
    }

    clique_cover_from_priority(g, C, P, T, O);
}

static inline int cmp_pri(const void *a, const void *b, void *p)
{
    int *P = (int *)p;
    node_id u = *((const node_id *)a), v = *((const node_id *)b);
    return P[v] - P[u];
}

void clique_cover_from_priority(graph *g, int *C, int *P, int *T, node_id *O)
{
    for (node_id u = 0; u < g->n; u++)
        C[u] = -1;

    for (node_id u = 0; u < g->n; u++)
        T[u] = 0;

    long long n = 0;
    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;
        O[n++] = u;
    }

    qsort_r(O, n, sizeof(node_id), cmp_pri, P);

    for (node_id i = 0; i < n; i++)
    {
        node_id u = O[i];

        for (node_id j = 0; j < g->D[u]; j++)
        {
            node_id v = g->V[u][j];
            if (C[v] >= 0)
                T[C[v]]--;
        }

        int c = 0;
        while (T[c] > 0)
            c++;

        C[u] = c;
        T[c]++;

        for (node_id j = 0; j < g->D[u]; j++)
        {
            node_id v = g->V[u][j];
            if (C[v] >= 0)
                T[C[v]]++;
        }
    }
}

int clique_cover_validate(graph *g, const int *C, int *T)
{
    for (node_id i = 0; i < g->n; i++)
        T[i] = 0;

    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;

        if (C[u] < 0)
            return 0;

        T[C[u]]++;
    }

    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;

        int count = 0, c = C[u];
        for (node_id i = 0; i < g->D[u]; i++)
        {
            node_id v = g->V[u][i];

            if (C[v] == c)
                count++;
        }
        if (count != T[c] - 1)
            return 0;
    }
    return 1;
}

node_weight clique_cover_upper_bound(graph *g, const int *C, node_id *M)
{
    node_weight ub = 0;
    int nc = 0;

    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;

        if (C[u] >= nc)
            nc = C[u] + 1;
    }

    for (int i = 0; i < nc; i++)
        M[i] = -1;

    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;

        int c = C[u];
        if (M[c] < 0 || (g->W[u] > g->W[M[c]]))
            M[c] = u;
    }

    for (int i = 0; i < nc; i++)
        ub += g->W[M[i]];

    return ub;
}