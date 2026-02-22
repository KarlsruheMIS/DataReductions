#pragma once

#include "defs.h"
#include "graph.h"
#include "reduction.h"

#include <stdarg.h>
#include <stdint.h>

typedef struct
{
    long long _a;

    int n_rules;
    reduction *Rules;
    long long *Rule_impact_V, *Rule_impact_E;

    node_id *Queue_count, **Queues;
    int8_t **In_queues;

    buffers *b;
    changed_list *c;

    int verbose;
    int log_impact;
} reducer;

typedef struct
{
    long long n, _a, offset;

    reconstruction_data *Log_data;
    reduction *Log_rule;

    long long *Graph_l;
    long long *Offsets;
} reduction_log;

reducer *reducer_init(graph *g, int n_rules, ...);

reducer *reducer_init_list(graph *g, int n_rules, const reduction *Rules);

void reducer_free(reducer *r);

void reducer_free_reduction_log(reduction_log *l);

int reducer_apply_reduction(graph *g, node_id u, reduction rule, reducer *r, reduction_log *l);

reduction_log *reducer_reduce(reducer *r, graph *g);

void reducer_reduce_continue(reducer *r, graph *g, reduction_log *l);

void reducer_include_vertex(reducer *r, graph *g, reduction_log *l, node_id u);

void reducer_exclude_vertex(reducer *r, graph *g, reduction_log *l, node_id u);

void reducer_restore_graph(graph *g, reduction_log *l, long long t);

void reducer_lift_solution(reduction_log *l, int *I);

reduction_log *reducer_init_reduction_log(graph *g);

void reducer_queue_all(reducer *r, graph *g);