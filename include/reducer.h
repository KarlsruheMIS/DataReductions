#pragma once

#include "defs.h"
#include "graph.h"
#include "reduction.h"

#include <stdarg.h>

typedef struct
{
    long long _a;

    int n_rules;
    reduction *Rule;

    node_id *Queue_count, **Queues;
    int **In_queues;

    buffers *b;
    change_list *c;
} reducer;

typedef struct
{
    long long n, _a;
    reconstruction_data *Log_data;
    reduction *Log_rule;
    long long *Offset;

    long long offset;
} reduction_log;

reducer *reducer_init(graph *g, int n_rules, ...);

void reducer_free(reducer *r);

void reducer_free_reduction_log(reduction_log *l);

reduction_log *reducer_reduce(reducer *r, graph *g);

void reducer_reduce_continue(reducer *r, graph *g, reduction_log *l);

void reducer_include_vertex(reducer *r, graph *g, reduction_log *l, node_id u);

void reducer_exclude_vertex(reducer *r, graph *g, reduction_log *l, node_id u);

void reducer_restore_graph(graph *g, reduction_log *l, long long t);

void reducer_lift_solution(reduction_log *l, int *I);