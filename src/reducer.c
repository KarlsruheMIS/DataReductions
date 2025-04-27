#include "reducer.h"

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
        }

        int res = r->Rule[rule].reduce(g, u, &l->Offset[l->n], r->b, r->c, &l->Log_data[l->n]);

        r->b->t++;

        // refactor this?
        if (r->b->t == (1 << 30))
        {
            r->b->t = 0;
            for (int i = 0; i < N_BUFFERS; i++)
                for (node_id j = 0; j < r->b->_a; j++)
                    r->b->fast_sets[i][j] = 0;
        }

        if (res)
        {
            while (g->_a > r->_a)
                reducer_increase(r);
            if (l->n == l->_a)
                reducer_increase_reduction_log(l);

            for (node_id i = 0; i < r->c->n; i++)
            {
                node_id v = r->c->V[i];
                for (int j = 0; j < r->n_rules; j++)
                {
                    if (!r->In_queues[j][v])
                    {
                        r->Queues[j][r->Queue_count[j]++] = v;
                        r->In_queues[j][v] = 1;
                    }
                }
            }
            l->Log_rule[l->n] = r->Rule[rule];
            l->offset += l->Offset[l->n];
            l->n++;
            rule = 0;
        }
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
}

void reducer_lift_solution(reduction_log *l, int *I)
{
}