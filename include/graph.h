#pragma once

#include "defs.h"

#include <stdio.h>

/*
    Dynamic graph structure for reductions

    For fast construction, use the add_vertex and add_edge functions
    When finished, call sort_edges to sort each neighborhood and
    remove duplicate edges. Once constructed, use the other methods
    (remove_edge, insert_edge, deactivate_vertex, etc.) to alter the
    graph while maintaining sorted neighborhoods.
*/

typedef struct
{
    long long n, m;  // Number of vertices and edges
    node_id **V, *D; // Adjacency lists and degrees
    node_weight *W;  // Vertex weights
    int *A;          // Active flags

    long long _a;  // Number of vertices allocated memory for
    long long *_A; // Allocated memory per neighborhood
} graph;

graph *graph_init();

graph *graph_parse(FILE *f);

void graph_free(graph *g);

// Functions for constructing a graph

void graph_sort_edges(graph *g);

void graph_add_vertex(graph *g, node_weight w);

void graph_add_edge(graph *g, node_id u, node_id v);

/*
    After construction, use these to maintain sorted neighborhoods.

    The deactivate function only removes edges from the active side,
    meaning the corresponding activate function can easily reconstruct
    the graph to before the deactivate call.
*/

void graph_remove_edge(graph *g, node_id u, node_id v);

void graph_insert_edge(graph *g, node_id u, node_id v);

void graph_deactivate_vertex(graph *g, node_id u);

void graph_activate_vertex(graph *g, node_id u);

void graph_deactivate_neighborhood(graph *g, node_id u);

void graph_activate_neighborhood(graph *g, node_id u);