#include "critical_set.h"
#include "algorithms.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>

node_weight tide_cycle(node_id n, node_id *E, node_id *ES, node_id *EL, node_id m,
                       node_id *R, node_weight *C, node_weight *P, node_weight *H, node_weight *L)
{
    assert(sizeof(node_weight) == sizeof(long long));

    for (node_id i = 0; i < n; i++)
        H[i] = 0;
    H[0] = LLONG_MAX;

    for (node_id i = 0; i < m; i++)
    {
        node_id e = EL[i];
        node_id u = ES[i], v = E[e];
        P[i] = (C[e] < H[u]) ? C[e] : H[u];
        H[v] += P[i];
    }

    if (H[n - 1] == 0)
        return 0;

    for (node_id i = 0; i < n; i++)
        L[i] = 0;
    L[n - 1] = H[n - 1];

    for (node_id i = m - 1; i >= 0; i--)
    {
        node_id e = EL[i];
        node_id u = ES[i], v = E[e];

        node_weight c = H[u] - L[u];
        P[i] = (c < P[i]) ? c : P[i];
        P[i] = (L[v] < P[i]) ? L[v] : P[i];

        L[v] -= P[i];
        L[u] += P[i];
    }

    for (node_id i = 0; i < n; i++)
        H[i] = 0;
    H[0] = L[0];

    for (node_id i = 0; i < m; i++)
    {
        node_id e = EL[i];
        node_id u = ES[i], v = E[e];

        P[i] = (H[u] < P[i]) ? H[u] : P[i];
        H[u] -= P[i];
        H[v] += P[i];
        C[e] -= P[i];
        C[R[e]] += P[i];
    }

    return H[n - 1];
}

void bfs(node_id n, node_id *V, node_id *E, node_weight *C, node_id *D,
         node_id *R, node_id *W, node_id *ES, node_id *EL, node_id *m)
{
    for (node_id i = 0; i < n; i++)
        D[i] = -1;

    node_id r = 1, w = 0;
    R[0] = 0;
    D[0] = 0;

    *m = 0;

    while (r > 0 && D[n - 1] < 0)
    {
        w = 0;
        for (node_id i = 0; i < r; i++)
        {
            node_id u = R[i];
            for (node_id j = V[u]; j < V[u + 1]; j++)
            {
                if (C[j] == 0)
                    continue;

                node_id v = E[j];
                if (D[v] < 0)
                {
                    W[w++] = v;
                    D[v] = D[u] + 1;
                }
                if (D[v] == D[u] + 1)
                {
                    ES[*m] = u;
                    EL[*m] = j;
                    (*m)++;
                }
            }
        }
        r = w;
        node_id *tmp = R;
        R = W;
        W = tmp;
    }
}

int critical_set_reduce_graph(graph *g, node_id u, node_weight *offset,
                              buffers *b, change_list *c, reconstruction_data *d)
{
    node_id *FM = malloc(sizeof(node_id) * g->n);
    node_id *RM = malloc(sizeof(node_id) * (g->n + 1));

    node_id n = 0, m = 0;

    for (node_id u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;

        n++;
        FM[u] = n;
        RM[n] = u;

        m += 4;
        m += g->D[u] * 2;
    }

    node_id na = n;

    n = n * 2 + 2;

    node_id *V = malloc(sizeof(node_id) * (n + 1));
    node_id *E = malloc(sizeof(node_id) * m);
    node_id *R = malloc(sizeof(node_id) * m);
    node_weight *C = malloc(sizeof(node_weight) * m);

    // Init offsets
    V[0] = 0;
    V[1] = 0;
    V[2] = na;
    for (node_id i = 1; i < na; i++)
        V[i + 2] = V[i + 1] + g->D[RM[i]] + 1;
    V[na + 2] = V[na + 1] + g->D[RM[na]] + 1;
    for (node_id i = na + 1; i < 2 * na; i++)
        V[i + 2] = V[i + 1] + g->D[RM[i - na]] + 1;

    V[n] = m - na;

    // Source vertex
    for (node_id i = 1; i <= na; i++)
    {
        node_id e = V[1]++, re = V[i + 1]++;

        E[e] = i;
        R[e] = re;
        C[e] = g->W[RM[i]];

        E[re] = 0;
        R[re] = e;
        C[re] = 0;
    }

    // First layer
    for (node_id i = 1; i <= na; i++)
    {
        node_id u = RM[i];
        for (node_id j = 0; j < g->D[u]; j++)
        {
            node_id v = g->V[u][j];

            node_id e = V[i + 1]++, re = V[FM[v] + na + 1]++;

            E[e] = FM[v] + na;
            R[e] = re;
            C[e] = g->W[u];

            E[re] = i;
            R[re] = e;
            C[re] = 0;
        }
    }

    // Second layer
    for (node_id i = na + 1; i <= 2 * na; i++)
    {
        node_id e = V[i + 1]++, re = V[n]++;

        E[e] = n - 1;
        R[e] = re;
        C[e] = g->W[RM[i - na]];

        E[re] = i;
        R[re] = e;
        C[re] = 0;
    }

    node_id *D = malloc(sizeof(node_id) * n);
    node_id *_R = malloc(sizeof(node_id) * n);
    node_id *_W = malloc(sizeof(node_id) * n);

    node_id *ES = malloc(sizeof(node_id) * m);
    node_id *EL = malloc(sizeof(node_id) * m);

    node_weight *P = malloc(sizeof(node_weight) * m);
    node_weight *H = malloc(sizeof(node_weight) * n);
    node_weight *L = malloc(sizeof(node_weight) * n);

    node_weight flow = 1;
    node_id ne = 0;
    node_id it_count = 0;
    while (flow > 0)
    {
        // for (node_id i = 0; i <= n; i++)
        //     printf("%d ", V[i]);
        // printf("\n");
        // for (node_id i = 0; i < m; i++)
        //     printf("%lld ", C[i]);
        // printf("\n");

        it_count++;
        bfs(n, V, E, C, D, _R, _W, ES, EL, &ne);
        if (D[n - 1] < 0)
            break;
        flow = tide_cycle(n, E, ES, EL, ne, R, C, P, H, L);
    }
    // printf("%d\n", it_count);

    bfs(n, V, E, C, D, _R, _W, ES, EL, &ne);

    node_id *red = malloc(sizeof(node_id) * na);
    node_id n_red = 0;
    for (node_id i = 1; i <= na; i++)
    {
        if (D[i] < 0)
            continue;

        red[n_red++] = RM[i];
    }
    if (n_red > 0)
    {
        d->n = n_red;
        d->data = (void *)red;
        *offset = 0;

        for (node_id i = 0; i < n_red; i++)
        {
            node_id u = red[i];
            *offset += g->W[u];
            graph_deactivate_neighborhood(g, u);
            reduction_data_queue_distance_two(g, u, c);
        }
    }
    else
    {
        free(red);
    }

    free(ES);
    free(EL);
    free(P);
    free(H);
    free(L);

    free(D);
    free(_R);
    free(_W);

    free(FM);
    free(RM);

    free(V);
    free(E);
    free(R);
    free(C);

    return n_red > 0;
}

void critical_set_restore_graph(graph *g, reconstruction_data *d)
{
    node_id *red = (node_id *)d->data;
    node_id n_red = d->n;

    for (node_id i = n_red - 1; i >= 0; i--)
    {
        node_id u = red[i];
        graph_activate_neighborhood(g, u);
    }
}

void critical_set_reconstruct_solution(int *I, reconstruction_data *d)
{
    node_id *red = (node_id *)d->data;
    node_id n_red = d->n;

    for (node_id i = 0; i < n_red; i++)
    {
        node_id u = red[i];
        I[u] = 1;
    }
}

void critical_set_clean(reconstruction_data *d)
{
    node_id *red = (node_id *)d->data;
    free(red);
}