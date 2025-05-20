#pragma once

#include <graph.h>

#define MOMENTUM 0.99
#define DECAY 0.8
#define FORCE_GRAVITY 0.1f
#define FORCE_PUSH 1.0f
#define PUSH_L 2.0f
#define LOCAL_L 8
#define SPRING_L 4.0f
#define SPRING_K 10.0f
#define EPSILON 0.000001f

#define INNER_DIM 64
#define OUTER_DIM 1536

typedef struct
{
    float weight;
    int locked;
    float x, y;
    float fx, fy;
    float vx, vy;
} vertex;

typedef struct
{
    double weight;
    float x, y;
} segment;

typedef struct
{
    int n, m;
    segment *G, *T, *Tf;
    vertex *V;
} force_layout;

force_layout *force_layout_init(graph *g);

void force_layout_free(force_layout *f);

void force_layout_step(force_layout *f, graph *g);