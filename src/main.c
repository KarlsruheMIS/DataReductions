#include "graph.h"
#include "reducer.h"
#include "degree_zero.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph *g = graph_parse(f);
    fclose(f);

    long long n = g->n, m = g->m;

    reducer *r = reducer_init(g, 1, degree_zero);
    reduction_log *l = reducer_reduce(r, g);

    long long n_reduced = 0;
    for (node_id i = 0; i < g->n; i++)
    {
        if (g->A[i])
            n_reduced++;
    }

    printf("%lld %lld -> %lld %lld (%lld)\n", n, m, n_reduced, g->m, l->offset);

    reducer_free_reduction_log(l);
    reducer_free(r);

    graph_free(g);

    return 0;
}