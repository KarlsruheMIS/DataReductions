#include "graph.h"
#include "reducer.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "domination.h"

#include <stdio.h>
#include <time.h>

double get_wtime()
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (double)tp.tv_sec + ((double)tp.tv_nsec / 1e9);
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph *g = graph_parse(f);
    fclose(f);

    double start = get_wtime();

    long long n = g->n, m = g->m;

    reducer *r = reducer_init(g, 3,
                              degree_zero,
                              degree_one,
                              domination);
    reduction_log *l = reducer_reduce(r, g);

    long long n_reduced = 0;
    for (node_id i = 0; i < g->n; i++)
    {
        if (g->A[i])
            n_reduced++;
    }

    int offset = 0, p = 0;
    while (argv[1][p] != '\0')
    {
        if (argv[1][p] == '/')
            offset = p + 1;
        p++;
    }

    double elapsed = get_wtime() - start;

    printf("%45s %10lld %10lld -> %10lld %10lld (%10lld) %3.4lf\n",
           argv[1] + offset, n, m, n_reduced, g->m, l->offset, elapsed);

    reducer_free_reduction_log(l);
    reducer_free(r);

    graph_free(g);

    return 0;
}