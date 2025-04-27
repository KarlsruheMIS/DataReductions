#include "graph.h"
#include "algorithms.h"

#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>

#define ALLOC_DEGREE 16

void graph_increase_size(graph *g)
{
    g->_a *= 2;
    g->V = realloc(g->V, sizeof(int *) * g->_a);
    g->D = realloc(g->D, sizeof(int) * g->_a);
    g->A = realloc(g->A, sizeof(int) * g->_a);
    g->_A = realloc(g->_A, sizeof(int) * g->_a);
    g->W = realloc(g->W, sizeof(long long) * g->_a);

    for (int i = g->_a / 2; i < g->_a; i++)
    {
        g->_A[i] = ALLOC_DEGREE;
        g->V[i] = malloc(sizeof(int) * g->_A[i]);
    }
}

void graph_append_endpoint(graph *g, int u, int v)
{
    if (g->_A[u] == g->D[u])
    {
        g->_A[u] *= 2;
        g->V[u] = realloc(g->V[u], sizeof(int) * g->_A[u]);
    }
    g->V[u][g->D[u]++] = v;
}

graph *graph_init()
{
    graph *g = malloc(sizeof(graph));
    *g = (graph){.n = 0, .m = 0, ._a = (1 << 10)};

    g->V = malloc(sizeof(int *) * g->_a);
    g->D = malloc(sizeof(int) * g->_a);
    g->A = malloc(sizeof(int) * g->_a);
    g->_A = malloc(sizeof(int) * g->_a);
    g->W = malloc(sizeof(long long) * g->_a);

    for (int i = 0; i < g->_a; i++)
    {
        g->_A[i] = ALLOC_DEGREE;
        g->V[i] = malloc(sizeof(int) * g->_A[i]);
    }

    return g;
}

static inline void parse_id(char *Data, size_t *p, long long *v)
{
    while (Data[*p] < '0' || Data[*p] > '9')
        (*p)++;

    *v = 0;
    while (Data[*p] >= '0' && Data[*p] <= '9')
        *v = (*v) * 10 + Data[(*p)++] - '0';
}

graph *graph_parse(FILE *f)
{
    graph *g = graph_init();

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *Data = mmap(0, size, PROT_READ, MAP_PRIVATE, fileno_unlocked(f), 0);
    size_t p = 0;

    long long n, m, w, t;
    parse_id(Data, &p, &n);
    parse_id(Data, &p, &m);
    parse_id(Data, &p, &t);

    for (int u = 0; u < n; u++)
    {
        graph_add_vertex(g, 0);
    }

    long long ei = 0;
    for (int u = 0; u < n; u++)
    {
        parse_id(Data, &p, &w);
        g->W[u] = w;

        while (ei < m * 2)
        {
            while (Data[p] == ' ')
                p++;
            if (Data[p] == '\n')
                break;

            long long e;
            parse_id(Data, &p, &e);
            if (e - 1 > u)
                graph_add_edge(g, u, e - 1);
            ei++;
        }
        p++;
    }

    munmap(Data, size);

    graph_sort_edges(g);
    return g;
}

void graph_free(graph *g)
{
    for (int i = 0; i < g->_a; i++)
    {
        free(g->V[i]);
    }
    free(g->V);
    free(g->D);
    free(g->A);
    free(g->_A);
    free(g->W);

    free(g);
}

void graph_sort_edges(graph *g)
{
    int m = 0;
    for (int i = 0; i < g->n; i++)
    {
        qsort(g->V[i], g->D[i], sizeof(int), compare_ints);
        int d = 0;
        for (int j = 0; j < g->D[i]; j++)
        {
            if (j == 0 || g->V[i][j] > g->V[i][d - 1])
                g->V[i][d++] = g->V[i][j];
        }
        g->D[i] = d;
        m += d;
    }
    g->m = m / 2;
}

void graph_add_vertex(graph *g, long long w)
{
    if (g->n == g->_a)
        graph_increase_size(g);

    int u = g->n;
    g->D[u] = 0;
    g->A[u] = 1;
    g->W[u] = w;
    g->n++;
}

void graph_add_edge(graph *g, int u, int v)
{
    assert(u < g->n && v < g->n);
    graph_append_endpoint(g, u, v);
    graph_append_endpoint(g, v, u);
    g->m += 1;
}

// Everything below this point assumes sorted neighborhoods

void graph_remove_endpoint(graph *g, int u, int v)
{
    int p = lower_bound(g->V[u], g->D[u], v);

    assert(p < g->D[u] && g->V[u][p] == v);

    for (int i = p + 1; i < g->D[u]; i++)
    {
        g->V[u][i - 1] = g->V[u][i];
    }
    g->D[u]--;
}

void graph_remove_endpoint_lin(graph *g, int u, int v)
{
    int p = -1;
    for (int i = 0; i < g->D[u]; i++)
    {
        if (g->V[u][i] == v)
        {
            p = i;
            break;
        }
    }

    assert(p >= 0);

    for (int i = p + 1; i < g->D[u]; i++)
    {
        g->V[u][i - 1] = g->V[u][i];
    }
    g->D[u]--;
}

void graph_remove_edge(graph *g, int u, int v)
{
    assert(g->A[u] && g->A[v]);

    graph_remove_endpoint_lin(g, u, v);
    graph_remove_endpoint_lin(g, v, u);
    g->m -= 1;
}

void graph_insert_endpoint(graph *g, int u, int v)
{
    int p = lower_bound(g->V[u], g->D[u], v);

    assert(p <= g->D[u] && (p == g->D[u] || g->V[u][p] > v));

    graph_append_endpoint(g, u, v);
    for (int i = p + 1; i < g->D[u]; i++)
    {
        g->V[u][i] = g->V[u][i - 1];
    }
    g->V[u][p] = v;
}

void graph_insert_endpoint_lin(graph *g, int u, int v)
{
    int p = 0;
    for (int i = 0; i < g->D[u]; i++)
    {
        if (g->V[u][i] > v)
            break;
        p++;
    }

    assert(p == g->D[u] || g->V[u][p] > v);

    graph_append_endpoint(g, u, v);
    for (int i = p + 1; i < g->D[u]; i++)
    {
        g->V[u][i] = g->V[u][i - 1];
    }
    g->V[u][p] = v;
}

void graph_insert_edge(graph *g, int u, int v)
{
    assert(g->A[u] && g->A[v]);

    graph_insert_endpoint_lin(g, u, v);
    graph_insert_endpoint_lin(g, v, u);
    g->m++;
}

void graph_deactivate_vertex(graph *g, int u)
{
    assert(g->A[u]);

    for (int i = 0; i < g->D[u]; i++)
    {
        int v = g->V[u][i];
        graph_remove_endpoint_lin(g, v, u);
    }
    g->A[u] = 0;
    g->m -= g->D[u];
}

void graph_activate_vertex(graph *g, int u)
{
    assert(!g->A[u]);

    for (int i = 0; i < g->D[u]; i++)
    {
        int v = g->V[u][i];
        graph_insert_endpoint_lin(g, v, u);
    }
    g->A[u] = 1;
    g->m += g->D[u];
}

void graph_deactivate_neighborhood(graph *g, int u)
{
    assert(g->A[u]);

    for (int i = 0; i < g->D[u]; i++)
    {
        int v = g->V[u][i];
        g->A[v] = 0;
    }
    g->A[u] = 0;

    int rm = g->D[u];
    for (int i = 0; i < g->D[u]; i++)
    {
        int v = g->V[u][i];
        for (int j = 0; j < g->D[v]; j++)
        {
            int w = g->V[v][j];
            rm++;
            if (!g->A[w])
                continue;
            graph_remove_endpoint_lin(g, w, v);
            rm += 2;
        }
    }
    g->m -= rm / 2;
}

void graph_activate_neighborhood(graph *g, int u)
{
    assert(g->A[u]);

    int rm = g->D[u];
    for (int i = 0; i < g->D[u]; i++)
    {
        int v = g->V[u][i];
        for (int j = 0; j < g->D[v]; j++)
        {
            int w = g->V[v][j];
            rm++;
            if (!g->A[w])
                continue;
            graph_insert_endpoint_lin(g, w, v);
            rm += 2;
        }
    }
    g->m += rm / 2;

    for (int i = 0; i < g->D[u]; i++)
    {
        int v = g->V[u][i];
        g->A[v] = 0;
    }
    g->A[u] = 0;
}