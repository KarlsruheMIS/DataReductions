#include "graph.h"
#include "reducer.h"
#include "algorithms.h"
#include "clique_cover.h"
#include "branch_and_reduce.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "domination.h"
#include "extended_domination.h"
#include "neighborhood_removal.h"
#include "simplicial_vertex.h"
#include "simplicial_vertex_with_weight_transfer.h"
#include "triangle.h"
#include "twin.h"
#include "unconfined.h"
#include "v_shape.h"
#include "weighted_funnel.h"
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

    long long n = g->n, m = g->m;

    reducer *r = reducer_init(g, 11,
                              degree_zero,
                              degree_one,
                              neighborhood_removal,
                              triangle,
                              v_shape,
                              domination,
                              twin,
                              simplicial_vertex_with_weight_transfer,
                              weighted_funnel,
                              unconfined,
                              extended_domination);

    double start = get_wtime();

    reduction_log *l = reducer_init_reduction_log(g);
    reducer_struction_fast(r, g, l, 6000);

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

    assert(g->n == n && g->m == m);

    free(I);

    reducer_free_reduction_log(l);
    reducer_free(r);

    graph_free(g);

    return 0;
}