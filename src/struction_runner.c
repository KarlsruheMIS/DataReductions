#define _GNU_SOURCE

#include "struction_runner.h"
#include "struction.h"
#include "algorithms.h"

#include <stdlib.h>

static inline int compare_by_degree(const void *a, const void *b, void *c)
{
    graph *g = (graph *)c;
    node_id u = *(const node_id *)a;
    node_id v = *(const node_id *)b;
    if (g->D[u] < g->D[v])
        return -1;
    if (g->D[u] > g->D[v])
        return 1;

    if (g->W[u] > g->W[v])
        return -1;
    return g->W[u] < g->W[v];
}

void struction_run(graph *g, reducer *r, reduction_log *l, int limit_m, int verbose)
{
    node_id *Order = malloc(sizeof(node_id) * g->n);

    long long n = 0;

    if (verbose)
        printf("Running struction %s\n", (limit_m ? "sparse" : "dense"));

    long long count = g->nr;

    while (count > g->nr / 100)
    {
        count = 0;

        n = 0;
        for (node_id u = 0; u < g->n; u++)
        {
            if (!g->A[u] || g->D[u] > MAX_STRUCTION_DEGREE)
                continue;

            Order[n++] = u;
        }

        qsort_r(Order, n, sizeof(node_id), compare_by_degree, (void *)g);

        for (long long i = 0; i < n; i++)
        {
            node_id u = Order[i];

            if (!g->A[u] || g->D[u] > MAX_STRUCTION_DEGREE)
                continue;

            if (!keep_running)
                break;

            long long n_ref = g->nr, m_ref = g->m, t = l->n;
            int res = reducer_apply_reduction(g, u, struction, r, l);

            if (!res)
                continue;

            reducer_reduce_continue(r, g, l);

            if (g->nr > n_ref || (limit_m && g->m > m_ref))
            {
                reducer_restore_graph(g, l, t);
                continue;
            }

            if (verbose)
            {
                printf("\r%10lld %10lld %20s", g->nr, g->m / 2, struction.name);
                fflush(stdout);
            }

            count += n_ref - g->nr;
        }
    }

    if (verbose)
    {
        printf("\r%10lld %10lld %20s\n", g->nr, g->m / 2, struction.name);
    }

    free(Order);
}
