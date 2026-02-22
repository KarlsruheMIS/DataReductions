#include "reducer.h"

#include "algorithms.h"

#include <stdlib.h>
#include <assert.h>

reducer *reducer_init_common(graph *g, int n_rules)
{
    reducer *r = malloc(sizeof(reducer));
    r->n_rules = n_rules;
    r->_a = g->_a;

    r->Rules = malloc(sizeof(reduction) * r->n_rules);
    r->Rule_impact_V = malloc(sizeof(long long) * r->n_rules);
    r->Rule_impact_E = malloc(sizeof(long long) * r->n_rules);

    r->Queue_count = malloc(sizeof(node_id) * r->n_rules);
    r->Queues = malloc(sizeof(node_id *) * r->n_rules);
    r->In_queues = malloc(sizeof(int8_t *) * r->n_rules);

    for (int i = 0; i < r->n_rules; i++)
    {
        r->Rule_impact_V[i] = 0;
        r->Rule_impact_E[i] = 0;
        r->Queue_count[i] = 0;
        r->Queues[i] = malloc(sizeof(node_id) * r->_a);
        r->In_queues[i] = malloc(sizeof(int8_t) * r->_a);

        for (long long j = 0; j < r->_a; j++)
            r->In_queues[i][j] = 0;
    }

    r->b = buffers_init(g);
    r->c = changed_list_init(g);

    r->verbose = 0;
    r->log_impact = 1;

    return r;
}

reducer *reducer_init(graph *g, int n_rules, ...)
{
    reducer *r = reducer_init_common(g, n_rules);

    va_list args;
    va_start(args, n_rules);

    for (int i = 0; i < r->n_rules; i++)
        r->Rules[i] = va_arg(args, reduction);

    va_end(args);

    return r;
}

reducer *reducer_init_list(graph *g, int n_rules, const reduction *Rules)
{
    reducer *r = reducer_init_common(g, n_rules);

    for (int i = 0; i < r->n_rules; i++)
        r->Rules[i] = Rules[i];

    return r;
}

void reducer_increase(reducer *r)
{
    r->_a *= 2;

    for (int i = 0; i < r->n_rules; i++)
    {
        r->Queues[i] = realloc(r->Queues[i], sizeof(node_id) * r->_a);
        r->In_queues[i] = realloc(r->In_queues[i], sizeof(int8_t) * r->_a);

        for (long long j = r->_a / 2; j < r->_a; j++)
        {
            r->In_queues[i][j] = 0;
        }
    }

    buffers_increase(r->b);
    changed_list_increase(r->c);
}

void reducer_free(reducer *r)
{
    free(r->Queue_count);

    for (int i = 0; i < r->n_rules; i++)
    {
        free(r->Queues[i]);
        free(r->In_queues[i]);
    }

    free(r->Queues);
    free(r->In_queues);

    free(r->Rules);
    free(r->Rule_impact_V);
    free(r->Rule_impact_E);

    buffers_free(r->b);
    changed_list_free(r->c);

    free(r);
}

reduction_log *reducer_init_reduction_log(graph *g)
{
    reduction_log *l = malloc(sizeof(reduction_log));

    l->n = 0;
    l->_a = g->_a;
    l->offset = 0ll;

    l->Log_data = malloc(sizeof(reconstruction_data) * l->_a);
    l->Log_rule = malloc(sizeof(reduction) * l->_a);
    l->Graph_l = malloc(sizeof(long long) * l->_a);
    l->Offsets = malloc(sizeof(long long) * l->_a);

    return l;
}

void reducer_increase_reduction_log(reduction_log *l)
{
    l->_a *= 2;

    l->Log_data = realloc(l->Log_data, sizeof(reconstruction_data) * l->_a);
    l->Log_rule = realloc(l->Log_rule, sizeof(reduction) * l->_a);
    l->Graph_l = realloc(l->Graph_l, sizeof(long long) * l->_a);
    l->Offsets = realloc(l->Offsets, sizeof(long long) * l->_a);
}

void reducer_free_reduction_log(reduction_log *l)
{
    for (int i = 0; i < l->n; i++)
    {
        l->Log_rule[i].clean(&l->Log_data[i]);
    }

    free(l->Log_data);
    free(l->Log_rule);
    free(l->Graph_l);
    free(l->Offsets);

    free(l);
}

void reducer_queue_changed(graph *g, reducer *r)
{
    for (node_id i = 0; i < r->c->n; i++)
    {
        node_id v = r->c->V[i];
        r->c->in_V[v] = 0;
        if (!g->A[v])
            continue;

        for (int j = 0; j < r->n_rules; j++)
        {
            if (!r->In_queues[j][v])
            {
                r->Queues[j][r->Queue_count[j]++] = v;
                r->In_queues[j][v] = 1;
            }
        }
    }
    r->c->n = 0;
}

int reducer_apply_reduction(graph *g, node_id u, reduction rule, reducer *r, reduction_log *l)
{
    l->Graph_l[l->n] = g->l;
    l->Offsets[l->n] = 0;
    int res = rule.reduce(g, u, l->Offsets + l->n, r->b, r->c, l->Log_data + l->n);
    r->b->t++;

    if (r->b->t == (1 << 30))
        buffers_reset_fast_sets(r->b);

    if (!res)
        return 0;

    while (g->_a > r->_a)
        reducer_increase(r);
    if (l->n + 1 == l->_a)
        reducer_increase_reduction_log(l);

    reducer_queue_changed(g, r);

    l->Log_rule[l->n] = rule;
    l->offset += l->Offsets[l->n];
    l->n++;

    return 1;
}

reduction_log *reducer_reduce(reducer *r, graph *g)
{
    reduction_log *l = reducer_init_reduction_log(g);
    reducer_queue_all(r, g);
    reducer_reduce_continue(r, g, l);
    return l;
}

void reducer_reduce_continue(reducer *r, graph *g, reduction_log *l)
{
    int rule = 0;

    if (r->verbose)
        printf("%10s %10s %20s\n", "Vertices", "Edges", "Reduction");

    int max_rule = 0;

    while (rule < r->n_rules && keep_running)
    {
        if (r->Queue_count[rule] == 0)
        {
            rule++;
            continue;
        }

        if (rule > max_rule)
        {
            max_rule = rule;
            if (r->verbose)
            {
                printf("\r%10lld %10lld %20s", g->nr, g->m / 2, r->Rules[max_rule].name);
                fflush(stdout);
            }
        }

        node_id u;
        if (r->Rules[rule].global)
        {
            u = 0;
            while (r->Queue_count[rule] > 0)
            {
                node_id v = r->Queues[rule][--r->Queue_count[rule]];
                r->In_queues[rule][v] = 0;
            }
        }
        else
        {
            u = r->Queues[rule][--r->Queue_count[rule]];
            r->In_queues[rule][u] = 0;

            if (!g->A[u])
                continue;
        }

        long long n = g->nr, m = g->m;

        int res = reducer_apply_reduction(g, u, r->Rules[rule], r, l);

        if (res)
        {
            if (r->log_impact)
            {
                r->Rule_impact_V[rule] += n - g->nr;
                r->Rule_impact_E[rule] += m - g->m;
            }
            if (r->verbose)
            {
                printf("\r%10lld %10lld %20s", g->nr, g->m / 2, r->Rules[max_rule].name);
                fflush(stdout);
            }
            rule = 0;
        }
    }
    if (r->verbose)
    {
        printf("\r%10lld %10lld %20s\n", g->nr, g->m / 2, r->Rules[max_rule].name);
    }
}

int include_vertex_reduce_graph(graph *g, node_id u, node_weight *offset,
                                buffers *b, changed_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    *offset = g->W[u];
    d->u = u;

    graph_remove_neighborhood(g, u);

    reduction_data_queue_distance_two(g, u, c);

    return 1;
}

void include_vertex_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 1;
}

void include_vertex_clean(reconstruction_data *d)
{
}

static reduction include_vertex = {
    .reduce = include_vertex_reduce_graph,
    .reconstruct = include_vertex_reconstruct_solution,
    .clean = include_vertex_clean,
    .global = 0,
    .name = "include_vertex",
};

void reducer_include_vertex(reducer *r, graph *g, reduction_log *l, node_id u)
{
    reducer_apply_reduction(g, u, include_vertex, r, l);
}

int exclude_vertex_reduce_graph(graph *g, node_id u, node_weight *offset,
                                buffers *b, changed_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    *offset = 0;
    d->u = u;

    graph_remove_vertex(g, u);

    reduction_data_queue_distance_one(g, u, c);

    return 1;
}

void exclude_vertex_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 0;
}

void exclude_vertex_clean(reconstruction_data *d)
{
}

static reduction exclude_vertex = {
    .reduce = exclude_vertex_reduce_graph,
    .reconstruct = exclude_vertex_reconstruct_solution,
    .clean = exclude_vertex_clean,
    .global = 0,
    .name = "exclude_vertex",
};

void reducer_exclude_vertex(reducer *r, graph *g, reduction_log *l, node_id u)
{
    reducer_apply_reduction(g, u, exclude_vertex, r, l);
}

void reducer_restore_graph(graph *g, reduction_log *l, long long t)
{
    while (l->n > t)
    {
        l->n--;
        graph_undo_changes(g, l->Graph_l[l->n]);
        l->Log_rule[l->n].clean(l->Log_data + l->n);
        l->offset -= l->Offsets[l->n];
    }
}

void reducer_lift_solution(reduction_log *l, int *I)
{
    for (long long i = l->n - 1; i >= 0; i--)
    {
        l->Log_rule[i].reconstruct(I, l->Log_data + i);
    }
}

void reducer_queue_all(reducer *r, graph *g)
{
    r->Queue_count[0] = 0;
    for (long long j = 0; j < g->n; j++)
    {
        if (!g->A[j])
            continue;

        r->Queues[0][r->Queue_count[0]++] = j;
        r->In_queues[0][j] = 1;
    }

    for (int i = 1; i < r->n_rules; i++)
    {
        r->Queue_count[i] = r->Queue_count[0];

        for (long long j = 0; j < r->Queue_count[0]; j++)
        {
            r->Queues[i][j] = r->Queues[0][j];
            r->In_queues[i][r->Queues[i][j]] = 1;
        }
    }
}