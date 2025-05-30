#include "branch_and_reduce.h"

#include "reducer.h"
#include "clique_cover.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "neighborhood_removal.h"
#include "triangle.h"
#include "v_shape.h"
#include "domination.h"
#include "twin.h"
#include "simplicial_vertex_with_weight_transfer.h"
#include "weighted_funnel.h"
#include "unconfined.h"
#include "extended_domination.h"

#include <stdlib.h>
#include <stdio.h>

void branch_and_reduce_req(graph *g, reducer *r, reduction_log *l,
                           node_weight *best_weight, int **best_I, long long *_n, int c)
{
    printf("\r%3d", c);
    fflush(stdout);
    if (g->nr == 0)
    {
        if (l->offset > *best_weight)
        {
            *best_weight = l->offset;
            if (g->n > *_n)
            {
                *_n = g->n;
                *best_I = realloc(*best_I, sizeof(int) * *_n);
            }
            for (node_id i = 0; i < g->n; i++)
                (*best_I)[i] = 0;
            reducer_lift_solution(l, *best_I);

            printf("\r%lld\n", *best_weight);
        }
        return;
    }

    node_id branch = -1;
    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;

        if (branch < 0 || (g->D[u] > g->D[branch] || g->D[u] == g->D[branch] && g->W[u] > g->W[branch]))
            branch = u;
    }

    clique_cover_largest_degree_first(g, r->b->buffers[0], r->b->buffers[1],
                                      r->b->buffers[2], r->b->buffers[3]);
    node_weight ub = clique_cover_upper_bound(g, r->b->buffers[0], r->b->buffers[1]);

    if (ub + l->offset <= *best_weight)
        return;

    long long t = l->n;
    // Include branch
    reducer_include_vertex(r, g, l, branch);
    reducer_reduce_continue(r, g, l, 60.0);
    branch_and_reduce_req(g, r, l, best_weight, best_I, _n, c + 1);

    reducer_restore_graph(g, l, t);

    // Exclude branch
    reducer_exclude_vertex(r, g, l, branch);
    reducer_reduce_continue(r, g, l, 60.0);
    branch_and_reduce_req(g, r, l, best_weight, best_I, _n, c + 1);

    reducer_restore_graph(g, l, t);
}

int *branch_and_reduce_solve(graph *g)
{
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

    reduction_log *l = reducer_reduce(r, g, 60.0);
    reducer_struction(r, g, l, 0, 60.0);

    int *I = NULL;
    long long _n = 0;
    node_weight cost = 6419;

    branch_and_reduce_req(g, r, l, &cost, &I, &_n, 0);

    printf("Finished, size = %lld\n", cost);

    free(I);

    reducer_restore_graph(g, l, 0);

    reducer_free_reduction_log(l);
    reducer_free(r);
}