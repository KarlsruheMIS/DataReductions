#include "barnes_hut.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

// --- Constants ---
const float EPS = 0.0001f;
const float MAX_FORCE = 1000.0f;

// --- Parameters ---
const float SOFTEN_EPS = 0.1f;
const float DECAY = 0.99f;

barnes_hut *barnes_hut_init(graph *g)
{
    barnes_hut *bh = malloc(sizeof(barnes_hut));

    bh->m = 0;
    bh->l = 0;

    int size = 1, width = GRID_WIDTH;
    while (width >= INNER_WIDTH)
    {
        bh->m += size;
        bh->l++;

        size *= 4;
        width /= 2;
    }

    bh->CX = malloc(sizeof(float) * bh->m);
    bh->CY = malloc(sizeof(float) * bh->m);
    bh->Mass = malloc(sizeof(float) * bh->m);
    bh->R = malloc(sizeof(float) * bh->m);
    bh->C = malloc(sizeof(int) * bh->m);
    bh->S = malloc(sizeof(int) * bh->m);

    size = 1, width = GRID_WIDTH;
    int count = 0;
    while (width >= INNER_WIDTH)
    {
        for (int i = count; i < count + size; i++)
            bh->S[i] = width;

        count += size;
        size *= 4;
        width /= 2;
    }

#pragma omp parallel
    {
#pragma omp single
        {
            bh->Queue = malloc(sizeof(int *) * omp_get_num_threads());
            bh->Queue_mark = malloc(sizeof(int *) * omp_get_num_threads());
        }
        bh->Queue[omp_get_thread_num()] = malloc(sizeof(int) * bh->m);
        bh->Queue_mark[omp_get_thread_num()] = malloc(sizeof(int) * bh->m);
    }

    bh->n = g->n;
    bh->X = malloc(sizeof(float) * g->n);
    bh->Y = malloc(sizeof(float) * g->n);
    bh->fX = malloc(sizeof(float) * g->n);
    bh->fY = malloc(sizeof(float) * g->n);
    bh->vX = malloc(sizeof(float) * g->n);
    bh->vY = malloc(sizeof(float) * g->n);
    bh->Tabu = malloc(sizeof(int) * g->n);

    for (int i = 0; i < g->n; i++)
    {
        bh->X[i] = (rand() % GRID_WIDTH);
        bh->Y[i] = (rand() % GRID_WIDTH);

        bh->fX[i] = 0.0f;
        bh->fY[i] = 0.0f;

        bh->vX[i] = 0.0f;
        bh->vY[i] = 0.0f;

        bh->Tabu[i] = 0;
    }

    bh->rest_l = 50.0f;
    bh->k_spring = 1.0f;
    bh->k_repel = 1.0f;
    bh->k_gravity = 1.0f;
    bh->theta = 1.0f;

    return bh;
}

void barnes_hut_free(barnes_hut *bh)
{
    free(bh->CX);
    free(bh->CY);
    free(bh->Mass);
    free(bh->R);
    free(bh->C);
    free(bh->S);

#pragma omp parallel
    {
        free(bh->Queue[omp_get_thread_num()]);
        free(bh->Queue_mark[omp_get_thread_num()]);
    }
    free(bh->Queue);
    free(bh->Queue_mark);

    free(bh->X);
    free(bh->Y);
    free(bh->fX);
    free(bh->fY);
    free(bh->vX);
    free(bh->vY);
    free(bh->Tabu);

    free(bh);
}

void barnes_hut_populate(barnes_hut *bh, graph *g)
{
    memset(bh->CX, 0, sizeof(float) * bh->m);
    memset(bh->CY, 0, sizeof(float) * bh->m);
    memset(bh->Mass, 0, sizeof(float) * bh->m);
    memset(bh->R, 0, sizeof(float) * bh->m);
    memset(bh->C, 0, sizeof(int) * bh->m);

    for (int u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;

        int p = 0;
        float x = bh->X[u], y = bh->Y[u];
        float A = 1.0f; // (float)(g->W[u]);
        float r = sqrtf(A / M_PI);

        for (int i = 0; i < bh->l; i++)
        {
            bh->CX[p] += bh->X[u] * A;
            bh->CY[p] += bh->Y[u] * A;
            bh->Mass[p] += A;
            bh->C[p] += 1;
            bh->R[p] += A * r;

            float w = bh->S[p] / 2;

            // Bot left
            if (x < w && y < w)
            {
                p = (p * 4) + 1;
            }
            // Top left
            else if (x < w)
            {
                p = (p * 4) + 2;
                y -= w;
            }
            // Bot right
            else if (y < w)
            {
                p = (p * 4) + 3;
                x -= w;
            }
            // Top right
            else
            {
                p = (p * 4) + 4;
                x -= w;
                y -= w;
            }
        }
    }
}

void barnes_hut_forces_repell(barnes_hut *bh, graph *g)
{

#pragma omp parallel
    {
        int *Queue = bh->Queue[omp_get_thread_num()];
        int *Queue_mark = bh->Queue_mark[omp_get_thread_num()];

#pragma omp for
        for (int u = 0; u < g->n; u++)
        {
            if (!g->A[u])
                continue;

            int s = 0, t = 0;
            Queue[t] = 0;
            Queue_mark[t] = 1;
            t++;

            float x = bh->X[u], y = bh->Y[u];
            float _x = bh->X[u], _y = bh->Y[u];
            float A = 1.0f; // (float)(g->W[u]);
            float r = sqrtf(A / M_PI);

            while (s < t)
            {
                int p = Queue[s];
                int w = bh->S[p] / 2;
                s++;

                float sum_x = bh->CX[p],
                      sum_y = bh->CY[p],
                      mass = bh->Mass[p],
                      ri = bh->R[p];

                int count = bh->C[p];

                if (Queue_mark[s])
                {
                    sum_x -= x * A;
                    sum_y -= y * A;
                    mass -= A;
                    count -= 1;
                    ri -= A * r;
                }

                if (count == 0)
                    continue;

                ri /= mass;

                float cx = sum_x / mass, cy = sum_y / mass;
                float dx = x - cx, dy = y - cy;
                float d = sqrtf(dx * dx + dy * dy) + EPS;

                float d_eff = d - (r + ri);
                d_eff = d_eff > 1.0f ? d_eff : 1.0f; // clamp

                if ((bh->S[p] / d_eff) < bh->theta || ((p * 4) + 1) >= bh->m)
                {
                    float force = (bh->k_repel * count) / d_eff;
                    bh->fX[u] += force * dx / d;
                    bh->fY[u] += force * dy / d;
                }
                else
                {
                    Queue_mark[t] = 0;
                    Queue_mark[t + 1] = 0;
                    Queue_mark[t + 2] = 0;
                    Queue_mark[t + 3] = 0;

                    // Bot left
                    if (_x < w && _y < w)
                    {
                        Queue_mark[t] = 1;
                    }
                    // Top left
                    else if (_x < w)
                    {
                        Queue_mark[t + 1] = 1;
                        _y -= w;
                    }
                    // Bot right
                    else if (_y < w)
                    {
                        Queue_mark[t + 2] = 1;
                        _x -= w;
                    }
                    // Top right
                    else
                    {
                        Queue_mark[t + 3] = 1;
                        _x -= w;
                        _y -= w;
                    }

                    Queue[t] = (p * 4) + 1;
                    Queue[t + 1] = (p * 4) + 2;
                    Queue[t + 2] = (p * 4) + 3;
                    Queue[t + 3] = (p * 4) + 4;

                    t += 4;
                }
            }
        }
    }
}

void barnes_hut_forces_spring(barnes_hut *bh, graph *g)
{
    float gx = GRID_WIDTH / 2, gy = GRID_WIDTH / 2;

#pragma omp parallel for
    for (int u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            continue;

        float x = bh->X[u], y = bh->Y[u];
        float ru = sqrtf(1.0f / M_PI); // sqrtf((float)g->W[u] / M_PI);

        float dx_g = gx - x,
              dy_g = gy - y;

        float d_g = sqrtf(dx_g * dx_g + dy_g * dy_g) + EPS;

        bh->fX[u] += (dx_g / d_g) * bh->k_gravity;
        bh->fY[u] += (dy_g / d_g) * bh->k_gravity;

        for (int i = 0; i < g->D[u]; i++)
        {
            int v = g->V[u][i];
            double rv = sqrt(1.0f / M_PI); // sqrt((float)g->W[v] / M_PI);
            if (v == u)
                continue;

            float dx = bh->X[v] - bh->X[u];
            float dy = bh->Y[v] - bh->Y[u];

            float d = sqrtf(dx * dx + dy * dy) + EPS;
            float rest_l = bh->rest_l + ru + rv;
            float s = bh->k_spring * (d - rest_l);

            bh->fX[u] += s * (dx / d) / (float)g->D[u];
            bh->fY[u] += s * (dy / d) / (float)g->D[u];
        }
    }
}

void barnes_hut_step(barnes_hut *bh, graph *g)
{
    for (int u = 0; u < g->n; u++)
    {
        bh->fX[u] = 0.0f;
        bh->fY[u] = 0.0f;
    }

    barnes_hut_forces_spring(bh, g);
    barnes_hut_populate(bh, g);
    barnes_hut_forces_repell(bh, g);

#pragma omp parallel for
    for (int u = 0; u < g->n; u++)
    {
        if (bh->Tabu[u] || !g->A[u])
        {
            bh->vX[u] = 0.0f;
            bh->vY[u] = 0.0f;
            continue;
        }

        bh->fX[u] = bh->fX[u] != bh->fX[u] ? 0.0f : bh->fX[u];
        bh->fY[u] = bh->fY[u] != bh->fY[u] ? 0.0f : bh->fY[u];

        bh->fX[u] = bh->fX[u] > MAX_FORCE ? MAX_FORCE : bh->fX[u];
        bh->fX[u] = bh->fX[u] < -MAX_FORCE ? -MAX_FORCE : bh->fX[u];

        bh->fY[u] = bh->fY[u] > MAX_FORCE ? MAX_FORCE : bh->fY[u];
        bh->fY[u] = bh->fY[u] < -MAX_FORCE ? -MAX_FORCE : bh->fY[u];

        bh->vX[u] = bh->vX[u] * (1.0f - SOFTEN_EPS) + bh->fX[u] * SOFTEN_EPS;
        bh->vY[u] = bh->vY[u] * (1.0f - SOFTEN_EPS) + bh->fY[u] * SOFTEN_EPS;

        bh->vX[u] *= DECAY;
        bh->vY[u] *= DECAY;

        bh->X[u] += bh->vX[u];
        bh->Y[u] += bh->vY[u];

        bh->X[u] = bh->X[u] >= GRID_WIDTH - 1 ? GRID_WIDTH - 1 : bh->X[u];
        bh->X[u] = bh->X[u] < 0.0f ? 0.0f : bh->X[u];

        bh->Y[u] = bh->Y[u] >= GRID_WIDTH - 1 ? GRID_WIDTH - 1 : bh->Y[u];
        bh->Y[u] = bh->Y[u] < 0.0f ? 0.0f : bh->Y[u];
    }
}