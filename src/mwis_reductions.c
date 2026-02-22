#include "mwis_reductions.h"

#include "defs.h"
#include "graph.h"
#include "reducer.h"
#include "struction_runner.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "neighborhood_removal.h"
#include "triangle.h"
#include "v_shape.h"
#include "domination.h"
#include "twin.h"
#include "simplicial_vertex.h"
#include "simplicial_vertex_with_weight_transfer.h"
#include "weighted_funnel.h"
#include "unconfined.h"
#include "extended_domination.h"
#include "critical_set.h"
#include "struction.h"

#include <stdio.h>
#include <stdlib.h>

volatile sig_atomic_t keep_running = 1;

static inline void update_mwis_reduction_graph(mwis_reduction_graph *mr_g, graph *g, reduction_log *l)
{
    *mr_g = (mwis_reduction_graph){
        .n = g->n,
        .m = g->m / 2,
        .nr = g->nr,
        .V = g->V,
        .D = g->D,
        .W = g->W,
        .A = g->A,
        .offset = 0,
        .internal_graph = (void *)g,
        .internal_reduction_log = NULL};

    if (l != NULL)
    {
        mr_g->offset = l->offset;
        mr_g->internal_reduction_log = (void *)l;
    }
}

mwis_reduction_graph *mwis_reduction_graph_read_from_file(const char *Path)
{
    FILE *f = fopen(Path, "r");
    if (f == NULL)
    {
        fprintf(stderr, "Failed to open file %s\n", Path);
        return NULL;
    }

    graph *g = graph_parse(f);

    mwis_reduction_graph *mr_g = malloc(sizeof(mwis_reduction_graph));

    update_mwis_reduction_graph(mr_g, g, NULL);

    *mr_g = (mwis_reduction_graph){
        .n = g->n,
        .m = g->m / 2,
        .nr = g->nr,
        .V = g->V,
        .D = g->D,
        .W = g->W,
        .A = g->A,
        .offset = 0,
        .internal_graph = (void *)g,
        .internal_reduction_log = NULL};

    fclose(f);

    return mr_g;
}

mwis_reduction_graph *mwis_reduction_graph_init()
{
    graph *g = graph_init();

    mwis_reduction_graph *mr_g = malloc(sizeof(mwis_reduction_graph));

    update_mwis_reduction_graph(mr_g, g, NULL);

    return mr_g;
}

void mwis_reduction_graph_add_vertex(mwis_reduction_graph *mr_g, node_weight w)
{
    graph *g = (graph *)mr_g->internal_graph;

    graph_add_vertex(g, w);

    *mr_g = (mwis_reduction_graph){
        .n = g->n,
        .m = g->m / 2,
        .nr = g->nr,
        .V = g->V,
        .D = g->D,
        .W = g->W,
        .A = g->A,
        .offset = 0,
        .internal_graph = (void *)g,
        .internal_reduction_log = NULL};
}

void mwis_reduction_graph_add_edge(mwis_reduction_graph *mr_g, node_id u, node_id v)
{
    graph *g = (graph *)mr_g->internal_graph;

    graph_add_edge(g, u, v);

    update_mwis_reduction_graph(mr_g, g, NULL);
}

void mwis_reduction_graph_free(mwis_reduction_graph *mr_g)
{
    graph *g = (graph *)mr_g->internal_graph;
    reduction_log *l = (reduction_log *)mr_g->internal_reduction_log;

    graph_free(g);
    if (l != NULL)
        reducer_free_reduction_log(l);

    free(mr_g);
}

reduction *mwis_reduction_convert_id_to_rules(int n_rules, reduction_t *Rule_IDs)
{
    reduction *Rules = malloc(sizeof(reduction) * n_rules);

    int n = 0;

    for (int i = 0; i < n_rules; i++)
    {
        switch (Rule_IDs[i])
        {
        case DEGREE_ZERO:
            Rules[n++] = degree_zero;
            break;
        case DEGREE_ONE:
            Rules[n++] = degree_one;
            break;
        case NEIGHBORHOOD_REMOVAL:
            Rules[n++] = neighborhood_removal;
            break;
        case TRIANGLE:
            Rules[n++] = triangle;
            break;
        case V_SHAPE:
            Rules[n++] = v_shape;
            break;
        case DOMINATION:
            Rules[n++] = domination;
            break;
        case TWIN:
            Rules[n++] = twin;
            break;
        case SIMPLICIAL_VERTEX:
            Rules[n++] = simplicial_vertex;
            break;
        case SIMPLICIAL_WEIGHT_TRANSFER:
            Rules[n++] = simplicial_vertex_with_weight_transfer;
            break;
        case FUNNEL:
            Rules[n++] = weighted_funnel;
            break;
        case UNCONFINED:
            Rules[n++] = unconfined;
            break;
        case EXTENDED_DOMINATION:
            Rules[n++] = extended_domination;
            break;
        case CRITICAL_SET:
            Rules[n++] = critical_set;
            break;

        default:
            fprintf(stderr, "Failed to parse reduction %d\n", Rule_IDs[i]);
            free(Rules);
            return NULL;
        }
    }

    return Rules;
}

void mwis_reduction_reduce_graph(mwis_reduction_graph *mr_g, int n_rules, reduction_t *Rule_IDs)
{
    graph *g = (graph *)mr_g->internal_graph;
    reduction_log *l = (reduction_log *)mr_g->internal_reduction_log;

    if (l == NULL)
        l = reducer_init_reduction_log(g);

    graph_construction_sort_edges(g);

    reduction *Rules = mwis_reduction_convert_id_to_rules(n_rules, Rule_IDs);

    if (Rules == NULL)
        return;

    reducer *r = reducer_init_list(g, n_rules, Rules);
    reducer_queue_all(r, g);

    reducer_reduce_continue(r, g, l);

    reducer_free(r);
    free(Rules);

    update_mwis_reduction_graph(mr_g, g, l);
}

void mwis_reduction_run_struction_sparse(mwis_reduction_graph *mr_g, int n_rules, reduction_t *Rule_IDs)
{
    graph *g = (graph *)mr_g->internal_graph;
    reduction_log *l = (reduction_log *)mr_g->internal_reduction_log;

    if (l == NULL)
        l = reducer_init_reduction_log(g);

    graph_construction_sort_edges(g);

    reduction *Rules = mwis_reduction_convert_id_to_rules(n_rules, Rule_IDs);

    if (Rules == NULL)
        return;

    reducer *r = reducer_init_list(g, n_rules, Rules);

    struction_run(g, r, l, 1, 0);

    reducer_free(r);
    free(Rules);

    update_mwis_reduction_graph(mr_g, g, l);
}

void mwis_reduction_run_struction_dense(mwis_reduction_graph *mr_g, int n_rules, reduction_t *Rule_IDs)
{
    graph *g = (graph *)mr_g->internal_graph;
    reduction_log *l = (reduction_log *)mr_g->internal_reduction_log;

    if (l == NULL)
        l = reducer_init_reduction_log(g);

    graph_construction_sort_edges(g);

    reduction *Rules = mwis_reduction_convert_id_to_rules(n_rules, Rule_IDs);

    if (Rules == NULL)
        return;

    reducer *r = reducer_init_list(g, n_rules, Rules);

    struction_run(g, r, l, 0, 0);

    reducer_free(r);
    free(Rules);

    update_mwis_reduction_graph(mr_g, g, l);
}

void mwis_reduction_restore_graph(mwis_reduction_graph *mr_g)
{
    graph *g = (graph *)mr_g->internal_graph;
    reduction_log *l = (reduction_log *)mr_g->internal_reduction_log;

    if (l == NULL)
        return;

    reducer_restore_graph(g, l, 0);

    update_mwis_reduction_graph(mr_g, g, l);
}

void mwis_reduction_lift_solution(mwis_reduction_graph *mr_g, int *I)
{
    graph *g = (graph *)mr_g->internal_graph;
    reduction_log *l = (reduction_log *)mr_g->internal_reduction_log;

    if (l == NULL)
        return;

    reducer_lift_solution(l, I);

    update_mwis_reduction_graph(mr_g, g, l);
}

void mwis_reduction_request_stop()
{
    keep_running = 0;
}
