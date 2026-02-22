#pragma once

#include "defs.h"
#include "graph.h"

#include <stdint.h>

typedef struct
{
    long long _a; // Number of elements allocated for (should always be >= to n)

    // Buffers
    node_id **buffers;
    node_weight **buffers_weigth;

    // Fast sets
    int t;
    int **fast_sets;
} buffers;

buffers *buffers_init(graph *g);

void buffers_increase(buffers *b);

void buffers_reset_fast_sets(buffers *b);

void buffers_free(buffers *b);

typedef struct
{
    long long _a; // Number of elements allocated for (should always be >= to n)

    node_id n;    // Number of vertices changed
    node_id *V;   // The changed vertices
    int8_t *in_V; // Set to 1 if vertex is in the list
} changed_list;

changed_list *changed_list_init(graph *g);

void changed_list_increase(changed_list *c);

void changed_list_free(changed_list *c);

typedef struct
{
    // Commonly used variables for most reductions
    node_id u, v, w, x, y, z;
    void *data; // For additional data (must be allocated manually)
} reconstruction_data;

// Queue up u and the neighborhood of u
void reduction_data_queue_distance_one(graph *g, node_id u, changed_list *c);

// Queue up u and the 2-hop neighborhood of u
void reduction_data_queue_distance_two(graph *g, node_id u, changed_list *c);