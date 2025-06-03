#include "reducer.h"

#include "algorithms.h"
#include "extended_struction.h"

#include <stdlib.h>
#include <assert.h>

#define STRUCTION_RULES 2

reducer *reducer_init(graph *g, int n_rules, ...)
{
    va_list args;
    va_start(args, n_rules);

    reducer *r = malloc(sizeof(reducer));
    r->n_rules = n_rules;
    r->_a = g->_a;

    r->Rule = malloc(sizeof(reduction) * r->n_rules);

    r->Queue_count = malloc(sizeof(node_id) * (r->n_rules + STRUCTION_RULES));
    r->Queues = malloc(sizeof(node_id *) * (r->n_rules + STRUCTION_RULES));
    r->In_queues = malloc(sizeof(int *) * (r->n_rules + STRUCTION_RULES));

    for (int i = 0; i < r->n_rules + STRUCTION_RULES; i++)
    {
        r->Queue_count[i] = g->n;
        r->Queues[i] = malloc(sizeof(node_id) * r->_a);
        r->In_queues[i] = malloc(sizeof(int) * r->_a);

        // Perhaps make filling the queues optional..
        for (long long j = 0; j < g->n; j++)
        {
            r->Queues[i][j] = j;
            r->In_queues[i][j] = 1;
        }
        for (long long j = g->n; j < r->_a; j++)
        {
            r->In_queues[i][j] = 0;
        }
    }

    for (int i = 0; i < r->n_rules; i++)
    {
        r->Rule[i] = va_arg(args, reduction);
    }

    reduction_data_init(g, &r->b, &r->c);

    va_end(args);

    r->verbose = 0;

    return r;
}

void reducer_increase(reducer *r)
{
    r->_a *= 2;

    for (int i = 0; i < r->n_rules + STRUCTION_RULES; i++)
    {
        r->Queues[i] = realloc(r->Queues[i], sizeof(node_id) * r->_a);
        r->In_queues[i] = realloc(r->In_queues[i], sizeof(int) * r->_a);

        for (long long j = r->_a / 2; j < r->_a; j++)
        {
            r->In_queues[i][j] = 0;
        }
    }

    reduction_data_increase(r->b, r->c);
}

void reducer_free(reducer *r)
{
    free(r->Queue_count);

    for (int i = 0; i < r->n_rules + STRUCTION_RULES; i++)
    {
        free(r->Queues[i]);
        free(r->In_queues[i]);
    }

    free(r->Queues);
    free(r->In_queues);

    free(r->Rule);
    reduction_data_free(r->b, r->c);

    free(r);
}

reduction_log *reducer_init_reduction_log(graph *g)
{
    reduction_log *l = malloc(sizeof(reduction_log));

    l->n = 0;
    l->offset = 0ll;

    l->_a = g->_a;
    l->Log_data = malloc(sizeof(reconstruction_data) * l->_a);
    l->Log_rule = malloc(sizeof(reduction) * l->_a);
    l->Offset = malloc(sizeof(long long) * l->_a);

    return l;
}

void reducer_increase_reduction_log(reduction_log *l)
{
    l->_a *= 2;

    l->Log_data = realloc(l->Log_data, sizeof(reconstruction_data) * l->_a);
    l->Log_rule = realloc(l->Log_rule, sizeof(reduction) * l->_a);
    l->Offset = realloc(l->Offset, sizeof(long long) * l->_a);
}

void reducer_free_reduction_log(reduction_log *l)
{
    for (int i = 0; i < l->n; i++)
    {
        l->Log_rule[i].clean(&l->Log_data[i]);
    }

    free(l->Log_data);
    free(l->Log_rule);
    free(l->Offset);

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

        for (int j = 0; j < r->n_rules + STRUCTION_RULES; j++)
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
    l->Offset[l->n] = 0;
    int res = rule.reduce(g, u, l->Offset + l->n, r->b, r->c, l->Log_data + l->n);
    r->b->t++;

    if (r->b->t == (1 << 30))
        reduction_data_reset_fast_sets(r->b);

    if (!res)
        return 0;

    while (g->_a > r->_a)
        reducer_increase(r);
    if (l->n == l->_a)
        reducer_increase_reduction_log(l);

    reducer_queue_changed(g, r);

    l->Log_rule[l->n] = rule;
    l->offset += l->Offset[l->n];
    l->n++;

    return 1;
}

reduction_log *reducer_reduce(reducer *r, graph *g, double tl)
{
    reduction_log *l = reducer_init_reduction_log(g);
    reducer_reduce_continue(r, g, l, tl);
    return l;
}

void reducer_reduce_continue(reducer *r, graph *g, reduction_log *l, double tl)
{
    int rule = 0;
    double t0 = get_wtime();
    while (rule < r->n_rules && get_wtime() - t0 < tl)
    {
        if (r->Queue_count[rule] == 0)
        {
            rule++;
            continue;
        }
        node_id u;
        if (r->Rule[rule].global)
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

        int res = reducer_apply_reduction(g, u, r->Rule[rule], r, l);

        if (res)
        {
            if (r->verbose)
            {
                printf("\r%10lld %10lld %10d", g->nr, g->m, rule);
                fflush(stdout);
            }
            rule = 0;
        }
    }
    if (r->verbose)
    {
        printf("\r%10lld %10lld %10d\n", g->nr, g->m, rule);
    }
}

void reducer_struction_fast(reducer *r, graph *g, reduction_log *l, double tl)
{
    // reducer_queue_all(r, g);

    int rule = r->n_rules;
    int verbose = r->verbose;
    r->verbose = 0;
    double t0 = get_wtime();
    reducer_reduce_continue(r, g, l, tl);
    while (rule < r->n_rules + 1 && get_wtime() - t0 < tl)
    {
        if (r->Queue_count[rule] == 0)
        {
            rule++;
            continue;
        }
        node_id u = r->Queues[rule][--r->Queue_count[rule]];
        r->In_queues[rule][u] = 0;

        if (!g->A[u])
            continue;

        long long n = g->nr, m = g->m, t = l->n,
                  r0 = r->Queue_count[r->n_rules], r1 = r->Queue_count[r->n_rules + 1];

        int res = reducer_apply_reduction(g, u, extended_struction, r, l);

        if (res)
        {
            reducer_reduce_continue(r, g, l, tl - (get_wtime() - t0));
            if (g->nr > n || (rule == r->n_rules && g->m > m))
            {
                reducer_restore_graph(g, l, t);
                res = 0;

                while (r->Queue_count[r->n_rules] > r0)
                {
                    node_id v = r->Queues[r->n_rules][--r->Queue_count[r->n_rules]];
                    r->In_queues[r->n_rules][v] = 0;
                }
                while (r->Queue_count[r->n_rules + 1] > r1)
                {
                    node_id v = r->Queues[r->n_rules + 1][--r->Queue_count[r->n_rules + 1]];
                    r->In_queues[r->n_rules + 1][v] = 0;
                }
            }
            if (res && verbose)
            {
                printf("\r%10lld %10lld %10d %10d %10.3lf", g->nr, g->m, rule, r->Queue_count[r->n_rules], get_wtime() - t0);
                fflush(stdout);
            }
            if (res)
                rule = r->n_rules;
        }
    }
    if (verbose)
    {
        printf("\r%10lld %10lld %10d\n", g->nr, g->m, rule);
    }
    r->verbose = verbose;
}

void reducer_struction(reducer *r, graph *g, reduction_log *l, int limit_m, double tl)
{
    // srand(time(NULL));

    int verbose = r->verbose;
    r->verbose = 0;

    double t0 = get_wtime(), t1 = get_wtime(), t2 = get_wtime();
    node_id max_d = 3;

    while (g->nr > 0 && t1 - t0 < tl && t1 - t2 < 1.0)
    {
        if (t1 - t2 > 0.5 && max_d < 16)
        {
            max_d++;
            t2 = get_wtime();
        }

        node_id u = rand() % g->n, _t = 0;
        while ((!g->A[u] || g->D[u] > max_d) && _t++ < 1000)
        {
            u = rand() % g->n;
        }

        if (!g->A[u] || g->D[u] > max_d)
        {
            t1 = get_wtime();
            continue;
        }

        long long n = g->nr, m = g->m, t = l->n, tg = g->l;

        int res = reducer_apply_reduction(g, u, extended_struction, r, l);

        if (!res)
        {
            t1 = get_wtime();
            continue;
        }

        // int it = rand() % 4;
        // for (int _t = 0; _t < it && r->Queue_count[0] > 0; _t++)
        // {
        //     node_id p = rand() % r->Queue_count[0];
        //     node_id v = r->Queues[0][p];
        //     if (!g->A[v])
        //         continue;

        //     res = reducer_apply_reduction(g, v, extended_struction, r, l);

        //     if (!res)
        //         continue;
        // }

        reducer_reduce_continue(r, g, l, tl - (t1 - t0));

        if (g->nr > n || (limit_m && g->m > m)) // || g->m > m
        {
            reducer_restore_graph(g, l, t);
        }
        else
        {
            t2 = get_wtime();
            if (verbose)
            {
                printf("\r%10lld %10lld %10.2lf %3d", g->nr, g->m, t1 - t0, max_d);
                fflush(stdout);
            }
        }
        t1 = get_wtime();
    }
    if (verbose)
        printf("\r%10lld %10lld %10.2lf\n", g->nr, g->m, t1 - t0);
    r->verbose = verbose;
}

int include_vertex_reduce_graph(graph *g, node_id u, node_weight *offset,
                                buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    *offset = g->W[u];
    d->u = u;
    d->n = g->l;

    graph_deactivate_neighborhood(g, u);

    reduction_data_queue_distance_two(g, u, c);

    return 1;
}

void include_vertex_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_undo_changes(g, d->n);
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
    .restore = include_vertex_restore_graph,
    .reconstruct = include_vertex_reconstruct_solution,
    .clean = include_vertex_clean,
    .global = 0,
};

void reducer_include_vertex(reducer *r, graph *g, reduction_log *l, node_id u)
{
    reducer_apply_reduction(g, u, include_vertex, r, l);
}

int exclude_vertex_reduce_graph(graph *g, node_id u, node_weight *offset,
                                buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    *offset = 0;
    d->u = u;
    d->n = g->l;

    graph_deactivate_vertex(g, u);

    reduction_data_queue_distance_one(g, u, c);

    return 1;
}

void exclude_vertex_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_undo_changes(g, d->n);
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
    .restore = exclude_vertex_restore_graph,
    .reconstruct = exclude_vertex_reconstruct_solution,
    .clean = exclude_vertex_clean,
    .global = 0,
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
        l->Log_rule[l->n].restore(g, l->Log_data + l->n);
        l->Log_rule[l->n].clean(l->Log_data + l->n);
        l->offset -= l->Offset[l->n];
    }
}

void reducer_lift_solution(reduction_log *l, int *I)
{
    for (long long i = l->n - 1; i >= 0; i--)
    {
        l->Log_rule[i].reconstruct(I, l->Log_data + i);
    }
}

void reducer_reduce_step(reducer *r, graph *g, reduction_log *l)
{
    int rule = 0;
    while (rule < r->n_rules)
    {
        if (r->Queue_count[rule] == 0)
        {
            rule++;
            continue;
        }
        node_id u;
        if (r->Rule[rule].global)
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

        int res = reducer_apply_reduction(g, u, r->Rule[rule], r, l);

        if (res)
        {
            return;
        }
    }
}

void reducer_queue_all(reducer *r, graph *g)
{
    for (int i = 0; i < r->n_rules + STRUCTION_RULES; i++)
    {
        r->Queue_count[i] = 0;

        for (long long j = 0; j < g->n; j++)
        {
            if (!g->A[j])
            {
                r->In_queues[i][j] = 0;
                continue;
            }
            r->Queues[i][r->Queue_count[i]++] = j;
            r->In_queues[i][j] = 1;
        }
        for (long long j = g->n; j < r->_a; j++)
        {
            r->In_queues[i][j] = 0;
        }
    }
}