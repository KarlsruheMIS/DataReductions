#include "graph.h"
#include "reducer.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "domination.h"
#include "neighborhood_removal.h"
#include "simplicial_vertex.h"
#include "simultaneous_set.h"
#include "triangle.h"
#include "twin.h"
#include "unconfined.h"
#include "v_shape.h"

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

    long long n = g->n, m = g->m;

    reducer *r = reducer_init(g, 9,
                              degree_zero,
                              degree_one,
                              neighborhood_removal,
                              triangle,
                              v_shape,
                              domination,
                              twin,
                              simplicial_vertex,
                              unconfined,
                              simultaneous_set);

    double start = get_wtime();
    reduction_log *l = reducer_reduce(r, g);
    double elapsed = get_wtime() - start;

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

    printf("%45s %10lld %10lld -> %10lld %10lld (%10lld) %8.4lf\n",
           argv[1] + offset, n, m, n_reduced, g->m, l->offset, elapsed);

    reducer_restore_graph(g, l, 0);

    reducer_free_reduction_log(l);
    reducer_free(r);

    graph_free(g);

    return 0;
}