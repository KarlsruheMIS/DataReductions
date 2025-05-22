#include "visualizer.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <immintrin.h>

force_layout *force_layout_init(graph *g)
{
    force_layout *f = malloc(sizeof(force_layout));

    f->n = g->_a;
    f->m = OUTER_DIM / INNER_DIM;

    f->G = malloc(sizeof(segment) * OUTER_DIM * OUTER_DIM);
    f->T = malloc(sizeof(segment) * f->m * f->m);
    f->Tf = malloc(sizeof(segment) * f->m * f->m);
    f->V = malloc(sizeof(vertex) * f->n);

    memset(f->G, 0, sizeof(segment) * OUTER_DIM * OUTER_DIM);
    memset(f->T, 0, sizeof(segment) * f->m * f->m);
    memset(f->Tf, 0, sizeof(segment) * f->m * f->m);
    memset(f->V, 0, sizeof(vertex) * f->n);

    for (int i = 0; i < f->n; i++)
    {
        f->V[i].x = ((double)rand() / (double)RAND_MAX) * (double)(OUTER_DIM - 1);
        f->V[i].y = ((double)rand() / (double)RAND_MAX) * (double)(OUTER_DIM - 1);
    }

    return f;
}

void force_layout_free(force_layout *f)
{
    free(f->G);
    free(f->T);
    free(f->Tf);
    free(f->V);
    free(f);
}

void force_layout_compute_local(segment *grid, int i, int j, float x, float y, float *fx, float *fy)
{
    if (i < 0)
        i = 0;
    else if (i + LOCAL_L >= OUTER_DIM)
        i = OUTER_DIM - LOCAL_L;

    if (j < 0)
        j = 0;
    else if (j + LOCAL_L >= OUTER_DIM)
        j = OUTER_DIM - LOCAL_L;

    for (int r = i; r < i + LOCAL_L; r++)
    {
        for (int c = j; c < j + LOCAL_L; c++)
        {
            segment *v = &grid[r * OUTER_DIM + c];
            float _fx = x - v->x, _fy = y - v->y;
            float d = sqrtf(_fx * _fx + _fy * _fy) + EPSILON;

            _fx /= d;
            _fy /= d;

            *fx += _fx * (1.0f / (d - PUSH_L > 0.1f ? d - PUSH_L : 0.1f)) * FORCE_PUSH * v->weight;
            *fy += _fy * (1.0f / (d - PUSH_L > 0.1f ? d - PUSH_L : 0.1f)) * FORCE_PUSH * v->weight;
        }
    }
}

void force_layout_compute_forces_global(force_layout *f, graph *g)
{
#pragma omp parallel for
    for (int i = 0; i < OUTER_DIM; i++)
    {
        memset(f->G + i * OUTER_DIM, 0, sizeof(segment) * OUTER_DIM);
    }

    memset(f->T, 0, sizeof(segment) * f->m * f->m);
    memset(f->Tf, 0, sizeof(segment) * f->m * f->m);

    // Fill the grid
#pragma omp parallel for
    for (int u = 0; u < f->n; u++)
    {
        if (u >= g->n || !g->A[u])
            continue;
        int x = roundf(f->V[u].x), y = roundf(f->V[u].y);
        f->G[y * OUTER_DIM + x].weight += 1.0f;
        f->G[y * OUTER_DIM + x].x += f->V[u].x;
        f->G[y * OUTER_DIM + x].y += f->V[u].y;
        // f->G[y * OUTER_DIM + x] += 1.0f; // Use weight instead
    }

    // Comput center of mass
    // TODO rephrase for par
#pragma omp parallel for
    for (int i = 0; i < OUTER_DIM; i++)
    {
        segment *t = f->T + (i / INNER_DIM) * f->m;
        segment *g = f->G + i * OUTER_DIM;
        for (int j = 0; j < OUTER_DIM; j++)
        {
            double v = g[j].weight;
            g[j].x /= v + EPSILON;
            g[j].y /= v + EPSILON;
            t[j / INNER_DIM].weight += v;
            t[j / INNER_DIM].x += g[j].x * v;
            t[j / INNER_DIM].y += g[j].y * v;
        }
    }

#pragma omp parallel for
    for (int i = 0; i < f->m; i++)
    {
        segment *t = f->T + i * f->m;
        for (int j = 0; j < f->m; j++)
        {
            t[j].x /= t[j].weight + EPSILON;
            t[j].y /= t[j].weight + EPSILON;
        }
    }

    // Find foreces on each block
#pragma omp parallel for
    for (int i = 0; i < f->m; i++)
    {
        for (int j = 0; j < f->m; j++)
        {
            segment *s = f->T + i * f->m + j;
            segment *sf = f->Tf + i * f->m + j;

            for (int i2 = 0; i2 < f->m; i2++)
            {
                for (int j2 = 0; j2 < f->m; j2++)
                {
                    float fx = s->x - f->T[i2 * f->m + j2].x,
                          fy = s->y - f->T[i2 * f->m + j2].y;
                    float d = sqrtf(fx * fx + fy * fy) + EPSILON;
                    float w = f->T[i2 * f->m + j2].weight;

                    fx /= d;
                    fy /= d;

                    sf->x += fx * (1.0f / (d - PUSH_L > 0.1f ? d - PUSH_L : 0.1f)) * FORCE_PUSH * w;
                    sf->y += fy * (1.0f / (d - PUSH_L > 0.1f ? d - PUSH_L : 0.1f)) * FORCE_PUSH * w;
                }
            }
        }
    }

    // Apply forces in each block
#pragma omp parallel for
    for (int u = 0; u < f->n; u++)
    {
        if (u >= g->n || !g->A[u])
            continue;
        int x = roundf(f->V[u].x), y = roundf(f->V[u].y);
        f->V[u].fx += f->Tf[(y / INNER_DIM) * f->m + x / INNER_DIM].x;
        f->V[u].fy += f->Tf[(y / INNER_DIM) * f->m + x / INNER_DIM].y;
    }

    // Apply local forces
#pragma omp parallel for
    for (int u = 0; u < f->n; u++)
    {
        if (u >= g->n || !g->A[u])
            continue;
        force_layout_compute_local(f->G, f->V[u].y - LOCAL_L / 2, f->V[u].x - LOCAL_L / 2,
                                   f->V[u].x, f->V[u].y,
                                   &f->V[u].fx, &f->V[u].fy);
    }
}

void force_layout_step(force_layout *f, graph *g)
{
    float c = OUTER_DIM / 2;

#pragma omp parallel for schedule(static, 256)
    for (int u = 0; u < g->n; u++)
    {
        if (u >= g->n || !g->A[u])
            continue;
        float fx = c - f->V[u].x, fy = c - f->V[u].y;
        float d = sqrtf(fx * fx + fy * fy) + EPSILON;

        f->V[u].fx = (d - 10.0f) * FORCE_GRAVITY * (fx / d);
        f->V[u].fy = (d - 10.0f) * FORCE_GRAVITY * (fy / d);
    }

    force_layout_compute_forces_global(f, g);

    // #pragma omp parallel for
    //     for (int u = 0; u < g->n; u++)
    //     {
    //         for (int v = 0; v < g->n; v++)
    //         {
    //             float fx = f->V[u].x - f->V[v].x, fy = f->V[u].y - f->V[v].y;
    //             float d = sqrtf(fx * fx + fy * fy) + EPSILON;

    //             fx /= d;
    //             fy /= d;

    //             f->V[u].fx += fx * (1.0f / (d - 3.0f > 0.1f ? d - 3.0f : 0.1f)) * FORCE_PUSH;
    //             f->V[u].fy += fy * (1.0f / (d - 3.0f > 0.1f ? d - 3.0f : 0.1f)) * FORCE_PUSH;
    //         }
    //     }

#pragma omp parallel for schedule(static, 256)
    for (int u = 0; u < g->n; u++)
    {
        if (u >= g->n || !g->A[u])
            continue;
        float deg = g->D[u] + 1.0f;
        for (int i = 0; i < g->D[u]; i++)
        {
            int v = g->V[u][i];
            if (!g->A[v])
                continue;
            float fx = f->V[v].x - f->V[u].x, fy = f->V[v].y - f->V[u].y;
            float d = sqrtf(fx * fx + fy * fy) + EPSILON;

            f->V[u].fx += ((d - SPRING_L) * SPRING_K * (fx / d)) / deg;
            f->V[u].fy += ((d - SPRING_L) * SPRING_K * (fy / d)) / deg;
        }
    }

#pragma omp parallel for schedule(static, 256)
    for (int u = 0; u < f->n; u++)
    {
        if (f->V[u].locked || u >= g->n ||  !g->A[u])
            continue;

        f->V[u].vx = MOMENTUM * f->V[u].vx + f->V[u].fx * (1.0f - MOMENTUM);
        f->V[u].vy = MOMENTUM * f->V[u].vy + f->V[u].fy * (1.0f - MOMENTUM);

        f->V[u].vx *= DECAY;
        f->V[u].vy *= DECAY;

        f->V[u].x += f->V[u].vx;
        f->V[u].y += f->V[u].vy;

        if (f->V[u].x > OUTER_DIM - 1)
            f->V[u].x = OUTER_DIM - 1;
        else if (f->V[u].x < 0.0f)
            f->V[u].x = 0.0f;

        if (f->V[u].y > OUTER_DIM - 1)
            f->V[u].y = OUTER_DIM - 1;
        else if (f->V[u].y < 0.0f)
            f->V[u].y = 0.0f;
    }
}