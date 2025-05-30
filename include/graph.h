#pragma once

#define __GRAPH__

#include "defs.h"

#include <stdio.h>

/*
    Dynamic graph structure for reductions

    For fast construction, use the graph_construction functions.
    When finished, call sort_edges to sort each neighborhood and
    remove duplicate edges. Once constructed, use the other methods
    (remove_edge, insert_edge, deactivate_vertex, etc.) to alter the
    graph while maintaining sorted neighborhoods.
*/

typedef struct
{
    long long n, m, nr; // Number of vertices, edges, and remaining vertices
    node_id **V, *D;    // Adjacency lists and degrees
    node_weight *W;     // Vertex weights
    int *A;             // Active flags

    long long _a;  // Number of vertices allocated memory for
    long long *_A; // Allocated memory per neighborhood

    long long l, _La; // Log counter and allocated memory
    void *Log;        // Log data
} graph;

graph *graph_init();

graph *graph_parse(FILE *f);

void graph_free(graph *g);

// Functions for constructing a graph

void graph_construction_add_vertex(graph *g, node_weight w);

void graph_construction_add_edge(graph *g, node_id u, node_id v);

void graph_construction_sort_edges(graph *g);

/*
    After construction, use these to maintain sorted neighborhoods.

    The deactivate function only removes edges from the active side,
    meaning the corresponding activate function can easily reconstruct
    the graph to before the deactivate call. Note that for this to work,
    activation calls must happen in the exact reverse order of
    the deactivation calls.
*/

void graph_add_vertex(graph *g, node_weight w);

void graph_add_edge(graph *g, node_id u, node_id v);

void graph_remove_edge(graph *g, node_id u, node_id v);

void graph_deactivate_vertex(graph *g, node_id u);

void graph_deactivate_neighborhood(graph *g, node_id u);

void graph_change_vertex_weight(graph *g, node_id u, node_weight w);

void graph_undo_changes(graph *g, long long t);

// Helper functions

int graph_is_neighbor(graph *g, node_id u, node_id v);