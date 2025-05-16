#include "reducer.h"

#include "algorithms.h"
#include "extended_struction.h"

#include <stdlib.h>

reducer *reducer_init(graph *g, int n_rules, ...)
{
    va_list args;
    va_start(args, n_rules);

    reducer *r = malloc(sizeof(reducer));
    r->n_rules = n_rules;
    r->_a = g->_a;

    r->Rule = malloc(sizeof(reduction) * r->n_rules);

    r->Queue_count = malloc(sizeof(node_id) * r->n_rules);
    r->Queues = malloc(sizeof(node_id *) * r->n_rules);
    r->In_queues = malloc(sizeof(int *) * r->n_rules);

    for (int i = 0; i < r->n_rules; i++)
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

        r->Rule[i] = va_arg(args, reduction);
    }

    reduction_data_init(g, &r->b, &r->c);

    va_end(args);

    return r;
}

void reducer_increase(reducer *r)
{
    r->_a *= 2;

    for (int i = 0; i < r->n_rules; i++)
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

    for (int i = 0; i < r->n_rules; i++)
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

reduction_log *reducer_reduce(reducer *r, graph *g)
{
    reduction_log *l = reducer_init_reduction_log(g);
    reducer_reduce_continue(r, g, l);
    return l;
}

void reducer_reduce_continue(reducer *r, graph *g, reduction_log *l)
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
            rule = 0;
    }
}

void reducer_struction(reducer *r, graph *g, reduction_log *l, int limit_m, double tl)
{
    // srand(time(NULL));

    double t0 = get_wtime(), t1 = get_wtime(), t2 = get_wtime();
    while (g->nr > 0 && t1 - t0 < tl && t1 - t2 < 1.0)
    {
        node_id u = rand() % g->n;
        while (!g->A[u])
            u = rand() % g->n;

        long long n = g->nr, m = g->m, t = l->n;

        int res = reducer_apply_reduction(g, u, extended_struction, r, l);

        if (!res)
            continue;

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

        reducer_reduce_continue(r, g, l);

        if (g->nr > n || (limit_m && g->m > m)) // || g->m > m
            reducer_restore_graph(g, l, t);
        else
        {
            t2 = get_wtime();
            printf("\r%10lld %10lld", g->nr, g->m);
            fflush(stdout);
        }
        t1 = get_wtime();
    }
}

void reducer_include_vertex(reducer *r, graph *g, reduction_log *l, node_id u)
{
}

void reducer_exclude_vertex(reducer *r, graph *g, reduction_log *l, node_id u)
{
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