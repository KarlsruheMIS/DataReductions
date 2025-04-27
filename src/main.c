#include "graph.h"
#include "reducer.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph *g = graph_parse(f);
    fclose(f);

    long long n = g->n, m = g->m;

    reducer *r = reducer_init(g, 0);
    reduction_log *l = reducer_reduce(r, g);

    printf("%lld %lld -> %lld %lld (%lld)\n", n, m, g->n, g->m, l->offset);

    reducer_free_reduction_log(l);
    reducer_free(r);

    graph_free(g);

    return 0;
}