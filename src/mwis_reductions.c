#include "graph.h"
#include "reducer.h"
#include "mwis_reductions.h"
#include "algorithms.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "triangle.h"
#include "v_shape.h"
#include "neighborhood_removal.h"
#include "simplicial_vertex_with_weight_transfer.h"
#include "twin.h"
#include "domination.h"
#include "critical_set.h"
#include "unconfined.h"
#include "extended_domination.h"
#include "weighted_funnel.h"
#include "critical_set.h"
#include "densify.h"
#include "struction.h"

#include <assert.h>
#include <stdlib.h>

typedef struct
{
    long long n_original;
    graph *g; // reduced but not remapped graph
    reduction_log *rl;
    int *forward_map;
    int *reverse_map;
} reduction_data;

reduction_data *build_reduced_graph(graph *g, reduction_log *l, long long orginal_size)
{
    node_id n_reduced = 0;
    graph *rg = graph_init();
    node_id *forward_map = malloc(sizeof(node_id) * g->n);
    node_id *reverse_map = malloc(sizeof(node_id) * g->n);
    for (node_id i = 0; i < g->n; i++)
    {
        if (g->A[i])
        {
            forward_map[i] = n_reduced;
            reverse_map[n_reduced] = i;
            n_reduced++;
            graph_construction_add_vertex(rg, g->W[i]);
        }
        else
        {
            forward_map[i] = g->n;
        }
        assert(n_reduced <= g->n);
    }
    reverse_map = realloc(reverse_map, sizeof(node_id) * n_reduced);

    // add all edges in reduced graph rg
    for (node_id u = 0; u < n_reduced; u++)
    {
        node_id ou = reverse_map[u]; // original graph vertex
        for (node_id j = 0; j < g->D[ou]; j++)
        {
            node_id v = forward_map[g->V[ou][j]];
            graph_construction_add_edge(rg, u, v);
        }
    }
    graph_construction_sort_edges(rg);

    reduction_data *rd = malloc(sizeof(reduction_data));
    *rd = (reduction_data){
        .n_original = orginal_size,
        .forward_map = forward_map,
        .reverse_map = reverse_map,
        .rl = l,
        .g = rg,
    };
    graph tmp = *rg;
    *rg = *g;
    *g = tmp;

    return rd;
}

void *mwis_reduction_reduce_graph(graph *g, double tl)
{
    graph_construction_sort_edges(g);
    long long orginal_size = g->n;
    reducer *r = reducer_init(g, 12,
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
                              extended_domination,
                              densify);

    reduction_log *l = reducer_reduce(r, g, tl);
    reducer_free(r);

    return (void *)build_reduced_graph(g, l, orginal_size);
}

void *mwis_reduction_run_struction(graph *g, double tl)
{
    double t0 = get_wtime();
    graph_construction_sort_edges(g);
    long long orginal_size = g->n;
    reducer *r = reducer_init(g, 11,
                              degree_zero,
                              degree_one,
                              neighborhood_removal,
                              triangle,
                              v_shape,
                              twin,
                              domination,
                              simplicial_vertex_with_weight_transfer,
                              weighted_funnel,
                              unconfined,
                              extended_domination);

    reduction_log *l = reducer_init_reduction_log(g);
    double t1 = get_wtime();
    double elapsed = t1 - t0;

    STRUCTION_MAX_DEGREE = 8;
    STRUCTION_MAX_NODES = 16;
    reducer_struction_fast(r, g, l, tl - (get_wtime() - t0));

    reducer_queue_all(r, g);
    STRUCTION_MAX_DEGREE = 8;
    STRUCTION_MAX_NODES = 25;
    reducer_struction_fast(r, g, l, tl - (get_wtime() - t0));

    reducer_queue_all(r, g);
    STRUCTION_MAX_DEGREE = 16;
    STRUCTION_MAX_NODES = 36;
    reducer_struction_fast(r, g, l, tl - (get_wtime() - t0));

    reducer_queue_all(r, g);
    STRUCTION_MAX_DEGREE = 16;
    STRUCTION_MAX_NODES = 49;
    reducer_struction_fast(r, g, l, tl - (get_wtime() - t0));

    reducer_queue_all(r, g);
    STRUCTION_MAX_DEGREE = 16;
    STRUCTION_MAX_NODES = 64;
    reducer_struction_fast(r, g, l, tl - (get_wtime() - t0));

    reducer_free(r);

    return (void *)build_reduced_graph(g, l, orginal_size);
}

void mwis_reduction_dinsify(graph *g, void *rd, double tl)
{
    reducer *r = reducer_init(g, 2, domination, densify);

    reducer_reduce_continue(r, g, ((reduction_data *)rd)->rl, tl);

    reducer_free(r);
}

int *mwis_reduction_lift_solution(int *rI, void *rd)
{
    reduction_data *d = (reduction_data *)rd;

    // get reduced solution on original graph
    int *I = malloc(sizeof(int) * d->g->n);
    for (node_id i = 0; i < d->g->n; i++)
    {
        if (!d->g->A[i])
            I[i] = 0;
        else
            I[i] = rI[d->forward_map[i]];
    }

    // lift solution by reconstructing the reductions
    for (int i = d->rl->n - 1; i >= 0; i--)
    {
        reduction rule = d->rl->Log_rule[i];
        rule.reconstruct(I, d->rl->Log_data + i);
    }

    I = realloc(I, sizeof(node_id) * d->n_original);
    return I;
}

void mwis_reduction_restore_graph(graph *rg, void *rd)
{
    reduction_data *d = (reduction_data *)rd;
    graph tmp = *rg;
    *rg = *d->g;
    *d->g = tmp;

    for (int i = d->rl->n - 1; i >= 0; i--)
    {
        reduction rule = d->rl->Log_rule[i];
        rule.restore(rg, d->rl->Log_data + i);
    }
}

void mwis_reduction_free(void *rd)
{
    reduction_data *d = (reduction_data *)rd;
    graph_free(d->g);
    reducer_free_reduction_log(d->rl);
    free(d->forward_map);
    free(d->reverse_map);
    free(d);
}

long long mwis_reduction_get_offset(void *rd)
{
    reduction_data *d = (reduction_data *)rd;
    return d->rl->offset;
}