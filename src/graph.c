#include "graph.h"
#include "algorithms.h"

#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>

#define ADD_VERTEX 0
#define INSERT_EDGE_ENDPOINT 1
#define REMOVE_EDGE_ENDPOINT 2
#define DEACTIVATE_VERTEX 3
#define DEACTIVATE_NEIGHBORHOOD 4
#define CHANGE_WEIGHT 5

typedef struct
{
    node_id u, v;
    node_weight w;
    int action;
} log_entry;

void graph_increase_size(graph *g)
{
    g->_a *= 2;
    g->V = realloc(g->V, sizeof(node_id *) * g->_a);
    g->D = realloc(g->D, sizeof(node_id) * g->_a);
    g->A = realloc(g->A, sizeof(int) * g->_a);
    g->_A = realloc(g->_A, sizeof(long long) * g->_a);
    g->W = realloc(g->W, sizeof(node_weight) * g->_a);

    for (int i = g->_a / 2; i < g->_a; i++)
    {
        g->_A[i] = ALLOC_DEGREE_INIT;
        g->V[i] = malloc(sizeof(node_id) * g->_A[i]);
    }
}

void graph_append_endpoint(graph *g, node_id u, node_id v)
{
    if (g->_A[u] == g->D[u])
    {
        g->_A[u] *= 2;
        g->V[u] = realloc(g->V[u], sizeof(node_id) * g->_A[u]);
    }
    g->V[u][g->D[u]++] = v;
}

graph *graph_init()
{
    graph *g = malloc(sizeof(graph));
    *g = (graph){.n = 0, .m = 0, .nr = 0, ._a = ALLOC_N_INIT, .l = 0, ._La = ALLOC_N_INIT};

    g->V = malloc(sizeof(node_id *) * g->_a);
    g->D = malloc(sizeof(node_id) * g->_a);
    g->A = malloc(sizeof(int) * g->_a);
    g->_A = malloc(sizeof(long long) * g->_a);
    g->W = malloc(sizeof(node_weight) * g->_a);

    for (int i = 0; i < g->_a; i++)
    {
        g->_A[i] = ALLOC_DEGREE_INIT;
        g->V[i] = malloc(sizeof(node_id) * g->_A[i]);
    }

    g->Log = (void *)malloc(sizeof(log_entry) * g->_La);

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

    for (node_id u = 0; u < n; u++)
    {
        graph_construction_add_vertex(g, 0);
    }

    long long ei = 0;
    for (node_id u = 0; u < n; u++)
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
                graph_construction_add_edge(g, u, e - 1);
            ei++;
        }
        p++;
    }

    munmap(Data, size);

    graph_construction_sort_edges(g);
    return g;
}

void graph_free(graph *g)
{
    for (node_id i = 0; i < g->_a; i++)
    {
        free(g->V[i]);
    }
    free(g->V);
    free(g->D);
    free(g->W);
    free(g->A);
    free(g->_A);
    free(g->Log);

    free(g);
}

void graph_construction_add_vertex(graph *g, node_weight w)
{
    if (g->n == g->_a)
        graph_increase_size(g);

    node_id u = g->n;
    g->D[u] = 0;
    g->A[u] = 1;
    g->W[u] = w;
    g->n++;
    g->nr++;
}

void graph_construction_add_edge(graph *g, node_id u, node_id v)
{
    assert(u < g->n && v < g->n);

    graph_append_endpoint(g, u, v);
    graph_append_endpoint(g, v, u);
}

void graph_construction_sort_edges(graph *g)
{
    long long m = 0;
    for (node_id i = 0; i < g->n; i++)
    {
        qsort(g->V[i], g->D[i], sizeof(node_id), compare_ids);
        int d = 0;
        for (node_id j = 0; j < g->D[i]; j++)
        {
            if (j == 0 || g->V[i][j] > g->V[i][d - 1])
                g->V[i][d++] = g->V[i][j];
        }
        g->D[i] = d;
        m += d;
    }
    g->m = m;
}

// Everything below this point assumes sorted neighborhoods

void graph_log_action(graph *g, node_id u, node_id v, node_weight w, node_id a)
{
    if (g->l >= g->_La)
    {
        g->_La *= 2;
        g->Log = realloc(g->Log, sizeof(log_entry) * g->_La);
    }

    ((log_entry *)g->Log)[g->l] = (log_entry){.u = u, .v = v, .w = w, .action = a};
    g->l++;
}

void graph_add_vertex(graph *g, node_weight w)
{
    graph_construction_add_vertex(g, w);
    graph_log_action(g, g->n - 1, 0, w, ADD_VERTEX);
}

int graph_insert_endpoint(graph *g, node_id u, node_id v, int log)
{
    node_id p = lower_bound(g->V[u], g->D[u], v);

    if (p < g->D[u] && g->V[u][p] == v)
        return 0;

    if (log)
        graph_log_action(g, u, v, 0, INSERT_EDGE_ENDPOINT);

    graph_append_endpoint(g, u, v);
    for (node_id i = g->D[u] - 1; i > p; i--)
    {
        g->V[u][i] = g->V[u][i - 1];
    }
    g->V[u][p] = v;
    g->m++;

    return 1;
}

void graph_add_edge(graph *g, node_id u, node_id v)
{
    assert(g->A[u] && g->A[v]);

    if (u == v)
        return;

    int e0 = graph_insert_endpoint(g, u, v, 1);
    int e1 = graph_insert_endpoint(g, v, u, 1);

    assert(e0 == e1);
}

int graph_remove_endpoint(graph *g, node_id u, node_id v, int log)
{
    node_id p = lower_bound(g->V[u], g->D[u], v);

    if (p >= g->D[u] || g->V[u][p] != v)
        return 0;

    if (log)
        graph_log_action(g, u, v, 0, REMOVE_EDGE_ENDPOINT);

    for (node_id i = p + 1; i < g->D[u]; i++)
    {
        g->V[u][i - 1] = g->V[u][i];
    }
    g->D[u]--;
    g->m--;

    return 1;
}

void graph_remove_edge(graph *g, node_id u, node_id v)
{
    assert(g->A[u] && g->A[v]);

    int e0 = graph_remove_endpoint(g, u, v, 1);
    int e1 = graph_remove_endpoint(g, v, u, 1);

    assert(e0 == e1);
}

void graph_deactivate_vertex(graph *g, node_id u)
{
    assert(g->A[u]);

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        graph_remove_endpoint(g, v, u, 0);
    }
    g->A[u] = 0;
    g->m -= g->D[u];
    g->nr--;

    graph_log_action(g, u, 0, 0, DEACTIVATE_VERTEX);
}

void graph_activate_vertex(graph *g, node_id u)
{
    assert(!g->A[u]);

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        graph_insert_endpoint(g, v, u, 0);
    }
    g->A[u] = 1;
    g->m += g->D[u];
    g->nr++;
}

void graph_deactivate_neighborhood(graph *g, node_id u)
{
    assert(g->A[u]);

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        g->A[v] = 0;
    }
    g->A[u] = 0;

    long long rm = g->D[u];
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        for (node_id j = 0; j < g->D[v]; j++)
        {
            node_id w = g->V[v][j];
            rm++;
            if (!g->A[w])
                continue;
            graph_remove_endpoint(g, w, v, 0);
        }
    }
    g->m -= rm;
    g->nr -= g->D[u] + 1;

    graph_log_action(g, u, 0, 0, DEACTIVATE_NEIGHBORHOOD);
}

void graph_activate_neighborhood(graph *g, node_id u)
{
    assert(!g->A[u]);

    long long rm = g->D[u];
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        for (node_id j = 0; j < g->D[v]; j++)
        {
            node_id w = g->V[v][j];
            rm++;
            if (!g->A[w])
                continue;
            graph_insert_endpoint(g, w, v, 0);
        }
    }
    g->m += rm;
    g->nr += g->D[u] + 1;

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];
        g->A[v] = 1;
    }
    g->A[u] = 1;
}

void graph_change_vertex_weight(graph *g, node_id u, node_weight w)
{
    graph_log_action(g, u, 0, g->W[u], CHANGE_WEIGHT);
    g->W[u] = w;
}

void graph_undo_changes(graph *g, long long t)
{
    while (g->l > t)
    {
        g->l--;
        log_entry e = ((log_entry *)g->Log)[g->l];

        switch (e.action)
        {
        case ADD_VERTEX:
            g->n--;
            g->nr--;
            break;
        case INSERT_EDGE_ENDPOINT:
            graph_remove_endpoint(g, e.u, e.v, 0);
            break;
        case REMOVE_EDGE_ENDPOINT:
            graph_insert_endpoint(g, e.u, e.v, 0);
            break;
        case DEACTIVATE_VERTEX:
            graph_activate_vertex(g, e.u);
            break;
        case DEACTIVATE_NEIGHBORHOOD:
            graph_activate_neighborhood(g, e.u);
            break;
        case CHANGE_WEIGHT:
            g->W[e.u] = e.w;
            break;
        default:
            break;
        }
    }
}

int graph_is_neighbor(graph *g, node_id u, node_id v)
{
    int pu = lower_bound(g->V[u], g->D[u], v);
    int uv = pu < g->D[u] && g->V[u][pu] == v;

    int pv = lower_bound(g->V[v], g->D[v], u);
    int vu = pv < g->D[v] && g->V[v][pv] == u;

    assert(uv == vu);

    return uv;
}