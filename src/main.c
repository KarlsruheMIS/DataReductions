#include "graph.h"
#include "reducer.h"
#include "algorithms.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "domination.h"
#include "extended_domination.h"
#include "neighborhood_removal.h"
#include "simplicial_vertex.h"
#include "simplicial_vertex_with_weight_transfer.h"
#include "simultaneous_set.h"
#include "triangle.h"
#include "twin.h"
#include "unconfined.h"
#include "v_shape.h"
#include "critical_set.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph *g = graph_parse(f);
    fclose(f);

    f = fopen(argv[1], "r");
    graph *copy = graph_parse(f);
    fclose(f);

    long long n = g->n, m = g->m;

    reducer *r = reducer_init(g, 10,
                              degree_zero,
                              degree_one,
                              neighborhood_removal,
                              triangle,
                              v_shape,
                              domination,
                              twin,
                              simplicial_vertex_with_weight_transfer,
                              extended_domination,
                              unconfined,
                              critical_set);

    double start = get_wtime();
    reduction_log *l = reducer_reduce(r, g);
    reducer_struction(r, g, l, 30);
    double elapsed = get_wtime() - start;

    int offset = 0, p = 0;
    while (argv[1][p] != '\0')
    {
        if (argv[1][p] == '/')
            offset = p + 1;
        p++;
    }

    printf("\r%45s %10lld %10lld %10lld %10lld %10lld %8.4lf\n",
           argv[1] + offset, n, m, g->nr, g->m, l->offset, elapsed);

    int *I = malloc(sizeof(int) * g->n);
    for (node_id i = 0; i < g->n; i++)
        I[i] = 0;

    reducer_lift_solution(l, I);
    reducer_restore_graph(g, l, 0);

    assert(g->n == n);
    for (node_id u = 0; u < g->n; u++)
    {
        assert(g->W[u] == copy->W[u]);
        assert(g->D[u] == copy->D[u]);
    }

    long long w = 0;
    for (node_id i = 0; i < copy->n; i++)
    {
        if (!I[i])
            continue;

        w += copy->W[i];
        for (node_id j = 0; j < copy->D[i]; j++)
        {
            if (I[copy->V[i][j]])
                printf("Error\n");
        }
    }

    // printf("%lld\n", w);

    // f = fopen("kernel.csv", "w");
    // fprintf(f, "source,target\n");
    // for (node_id u = 0; u < g->n; u++)
    // {
    //     if (!g->A[u])
    //         continue;

    //     for (node_id i = 0; i < g->D[u]; i++)
    //     {
    //         node_id v = g->V[u][i];
    //         fprintf(f, "%d,%d\n", u, v);
    //     }
    // }
    // fclose(f);

    // f = fopen("kernel_meta.csv", "w");
    // fprintf(f, "id,weight\n");
    // for (node_id u = 0; u < g->n; u++)
    // {
    //     if (!g->A[u])
    //         continue;

    //     fprintf(f, "%d,%lld\n", u, g->W[u]);
    // }
    // fclose(f);

    free(I);

    reducer_free_reduction_log(l);
    reducer_free(r);

    graph_free(g);
    graph_free(copy);

    return 0;
}