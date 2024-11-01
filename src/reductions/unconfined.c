#include "reductions.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

#define MAX_DEGREE 512
#define MAX_SOLVE 128

int reduction_unconfined_csr(reduction_data *R, int N, const int *V, const int *E,
                             const long long *W, const int *A, int u, int *nRed, int *reducable)
{
    assert(A[u]);

    reduction_data *rp = (reduction_data *)R;

    int n = 0, m = 0;

    assert(rp->Nb >= 3);

    int *S = rp->T[0], *NS = rp->T[1];
    int *S_B = rp->TB[0], *NS_B = rp->TB[1], *NIS_B = rp->TB[2];

    S[n++] = u;
    S_B[u] = 1;
    NIS_B[u] = 1;
    for (int i = V[u]; i < V[u + 1] && m <= MAX_DEGREE; i++)
    {
        int v = E[i];
        if (!A[v])
            continue;
        NS[m++] = v;
        NS_B[v] = 1;
        NIS_B[v] = 1;
    }

    int res = 0, first = 1;
    while (m > 0 && m <= MAX_DEGREE)
    {
        int v = NS[--m];
        NS_B[v] = 0;
        if (S_B[v])
            continue;

        long long sw = 0, dw = 0;
        int dn = 0, x = -1;

        if (first && W[v] < W[u]) // Not a child
            continue;

        for (int i = V[v]; i < V[v + 1]; i++)
        {
            int w = E[i];
            if (!A[w])
                continue;

            if (S_B[w])
                sw += W[w];
            else if (!NIS_B[w])
            {
                dw += W[w];
                dn++;
                x = w;
            }
        }

        if (W[v] < sw) // Not a chld
            continue;

        if (sw + dw <= W[v]) // Condition 1. can reduce u
        {
            res = 1;
            break;
        }
        else if (sw + dw > W[v] && dn == 1) // Extending child
        {
            first = 0;
            S[n++] = x;
            S_B[x] = 1;
            NIS_B[x] = 1;
            for (int i = V[x]; i < V[x + 1]; i++)
            {
                int w = E[i];
                if (!A[w] || NS_B[w])
                    continue;
                NS[m++] = w;
                NS_B[w] = 1;
                NIS_B[w] = 1;
            }
        }
    }

    while (m > 0)
        NS_B[NS[--m]] = 0;

    for (int i = 0; i < n; i++)
    {
        int v = S[i];
        for (int j = V[v]; j < V[v + 1]; j++)
            NIS_B[E[j]] = 0;
        S_B[v] = 0;
        NIS_B[v] = 0;
    }

    if (res)
    {
        *nRed = 1;
        reducable[0] = u;
        return -1;
    }
    return 0;
}

int reduction_extended_unconfined_csr(reduction_data *R, int N, const int *V, const int *E,
                                      const long long *W, const int *A, int u, int *nRed, int *reducable)
{
    assert(A[u]);

    int n = 0, m = 0;

    assert(R->Nb >= 4);

    int *S = R->T[0], *NS = R->T[1], *T = R->T[2], *I = R->T[3];
    int *S_B = R->TB[0], *NS_B = R->TB[1], *NIS_B = R->TB[2];

    S[n++] = u;
    S_B[u] = 1;
    NIS_B[u] = 1;
    for (int i = V[u]; i < V[u + 1] && m <= MAX_DEGREE; i++)
    {
        int v = E[i];
        if (!A[v])
            continue;
        NS[m++] = v;
        NS_B[v] = 1;
        NIS_B[v] = 1;
    }

    int res = 0, first = 1;
    while (m > 0 && m <= MAX_DEGREE)
    {
        int v = NS[--m];
        NS_B[v] = 0;
        if (S_B[v])
            continue;

        long long sw = 0, dw = 0, dsw = 0;
        int dn = 0, in = 0;

        if (first && W[v] < W[u]) // Not a child
            continue;

        for (int i = V[v]; i < V[v + 1]; i++)
        {
            int w = E[i];
            if (!A[w])
                continue;

            if (S_B[w])
                sw += W[w];
            else if (!NIS_B[w])
            {
                dw += W[w];
                T[dn++] = w;
            }
        }

        if (W[v] < sw || dn > MAX_SOLVE) // Not a chld
            continue;

        if (W[v] >= sw + dw) // Can reduce u
        {
            res = 1;
            break;
        }

        tiny_solver_solve_subgraph(R->solver, 1.0, W[v] - sw, dn, T, V, E, W, A);
        if (R->solver->time_limit_exceeded)
            continue;

        dw = R->solver->independent_set_weight;
        dsw = 0;

        if (W[v] >= sw + dw) // Can reduce u
        {
            res = 1;
            break;
        }

        // Copy condidates
        for (int i = 0; i < dn; i++)
            I[i] = R->solver->independent_set[i] == 1 ? 1 : 0;

        // Find vertices to extend S, solve without each vertex included
        for (int i = 0; i < dn; i++)
        {
            if (!I[i])
                continue;

            int w = T[i];
            T[i] = T[dn - 1];
            tiny_solver_solve_subgraph(R->solver, 1.0, W[v] - sw, dn - 1, T, V, E, W, A);

            if (!R->solver->time_limit_exceeded && W[v] >= sw + R->solver->independent_set_weight)
            {
                first = 0;

                S[n++] = w;
                S_B[w] = 1;
                NIS_B[w] = 1;
                for (int j = V[w]; j < V[w + 1]; j++)
                {
                    int x = E[j];
                    if (!A[x] || NS_B[x])
                        continue;
                    NS[m++] = x;
                    NS_B[x] = 1;
                    NIS_B[x] = 1;
                }
            }

            T[i] = w;
        }
    }

    while (m > 0)
    {
        int v = NS[--m];
        NS_B[v] = 0;
    }

    for (int i = 0; i < n; i++)
    {
        int v = S[i];
        for (int j = V[v]; j < V[v + 1]; j++)
            NIS_B[E[j]] = 0;
        S_B[v] = 0;
        NIS_B[v] = 0;
    }

    if (res)
    {
        *nRed = 1;
        reducable[0] = u;
        return -1;
    }

    return 0;
}

int reduction_boolean_width_csr(reduction_data *R, int N, const int *V, const int *E,
                                const long long *W, const int *A, int u, int *nRed, int *reducable)
{
    assert(A[u]);
    assert(R->Nb >= 3);

    long long Wx = 0, Wy = 0;

    int *Sy = R->T[0], *Sx = R->T[1];
    int *SBy = R->TB[0], *SBx = R->TB[1];

    int *NBu = R->TB[2];
    NBu[u] = 1;

    for (int i = V[u]; i < V[u + 1]; i++)
    {
        int v = E[i];
        if (!A[v])
            continue;

        NBu[v] = 1;
    }

    int x = -1;
    long long wy;
    for (int i = V[u]; i < V[u + 1]; i++)
    {
        int _x = E[i];
        if (!A[_x])
            continue;

        long long _wy = 0;
        for (int j = V[_x]; j < V[_x + 1]; j++)
        {
            int v = E[j];
            if (!A[v] || NBu[v])
                continue;

            _wy += W[v];
        }

        if (x < 0 || W[x] - wy < W[_x] - _wy)
        {
            x = _x;
            wy = _wy;
        }
    }

    if (x < 0)
        return 0;

    int ny = 0, nx = 0;

    Wy += wy;
    Wx += W[x];
    Sx[nx++] = x;
    SBx[x] = 1;

    for (int i = V[x]; i < V[x + 1]; i++)
    {
        int v = E[i];
        if (!A[v] || NBu[v])
            continue;

        Sy[ny++] = v;
        SBy[v] = 1;
    }

    // printf("%lld >= %lld + %lld (%lld)\n", Wx, W[u], Wy, Wx - (W[u] + Wy));

    while (1)
    {
        x = -1;
        wy = 0;
        for (int i = V[u]; i < V[u + 1]; i++)
        {
            int _x = E[i];
            if (!A[_x] || SBx[_x])
                continue;

            long long _wy = 0;
            int cand = 1;
            for (int j = V[_x]; j < V[_x + 1]; j++)
            {
                int v = E[j];
                if (!A[v])
                    continue;

                if (SBx[v])
                {
                    cand = 0;
                    break;
                }

                if (NBu[v] || SBy[v])
                    continue;

                _wy += W[v];
            }

            if (cand && (x < 0 || W[x] - wy < W[_x] - _wy))
            {
                x = _x;
                wy = _wy;
            }
        }

        if (x < 0 || W[x] - wy < 0)
            break;

        Wy += wy;
        Wx += W[x];
        Sx[nx++] = x;
        SBx[x] = 1;

        for (int i = V[x]; i < V[x + 1]; i++)
        {
            int v = E[i];
            if (!A[v] || NBu[v] || SBy[v])
                continue;

            Sy[ny++] = v;
            SBy[v] = 1;
        }

        // printf("%lld >= %lld + %lld (%lld)\n", Wx, W[u], Wy, Wx - (W[u] + Wy));

        if (Wx >= W[u] + Wy)
            break;
    }

    // if (ny > 0 && ny < 128)
    // {
    //     tiny_solver_solve_subgraph(R->solver, 10.0, 999999999, ny, Sy, V, E, W, A);
    //     //printf("%lld %lld\n", Wy, R->solver->independent_set_weight);
    //     if (R->solver->time_limit_exceeded)
    //         Wy = R->solver->independent_set_weight;
    // }

    // printf("\n");

    NBu[u] = 0;
    for (int i = V[u]; i < V[u + 1]; i++)
    {
        int v = E[i];
        if (!A[v])
            continue;

        NBu[v] = 0;

        if (!SBx[v])
            continue;

        SBx[v] = 0;

        for (int j = V[v]; j < V[v + 1]; j++)
        {
            int w = E[j];
            if (!A[w])
                continue;

            SBy[w] = 0;
        }
    }

    // for (int i = 0; i < N; i++)
    // {
    //     SBx[i] = 0;
    //     SBy[i] = 0;
    //     NBu[i] = 0;
    // }

    if (Wx >= W[u] + Wy)
    {
        *nRed = 1;
        reducable[0] = u;
        return -1;
    }

    return 0;
}

int reduction_boolean_width_alt_csr(reduction_data *R, int N, const int *V, const int *E,
                                    const long long *W, const int *A, int u, int *nRed, int *reducable)
{
    assert(A[u]);

    tiny_solver_solve_neighbourhood(R->solver, 10.0, 999999999, u, V, E, W, A);
    if (R->solver->time_limit_exceeded || R->solver->node_limit_exceeded)
        return 0;

    long long Wx = R->solver->independent_set_weight;

    if (Wx <= W[u])
    {
        *nRed = 1;
        reducable[0] = u;
        return 1;
    }

    assert(R->Nb >= 3);

    int nx = 0, ny = 0;
    int *Sx = R->T[0], *Sy = R->T[1];
    int *SBx = R->TB[0], *SBy = R->TB[1], *NBu = R->TB[2];

    for (int i = V[u]; i < V[u + 1]; i++)
    {
        int v = E[i];
        if (!A[v])
            continue;
        NBu[v] = 1;
    }

    long long Wy = 0;
    for (int i = 0; i < MAX_SOLVE; i++)
    {
        if (R->solver->independent_set[i] == 1)
        {
            int v = R->solver->reverse_map[i];
            Sx[nx++] = v;
            SBx[v] = 1;
        }
    }
    for (int i = 0; i < nx; i++)
    {
        int v = Sx[i];
        for (int j = V[v]; j < V[v + 1]; j++)
        {
            int w = E[j];
            if (!A[w] || SBy[w] || NBu[w])
                continue;

            Wy += W[w];
            Sy[ny++] = w;
            SBy[w] = 1;
        }
    }

    // Can reduce Wy by solving MWIS in Sy

    for (int i = 0; i < N; i++)
    {
        SBx[i] = 0;
        SBy[i] = 0;
        NBu[i] = 0;
    }

    if (Wx >= Wy)
    {
        *nRed = 1;
        reducable[0] = u;
        return -1;
    }

    return 0;
}