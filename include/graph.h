#pragma once

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
    int n, m;        // Number of vertices and edges
    int **V, *D, *A; // Adjacency lists, degrees, and active flags
    long long *W;    // Vertex weights

    int _a;  // Number of vertices allocated memory for
    int *_A; // Allocated memory per neighborhood
} graph;

graph *graph_init();

graph *graph_parse(FILE *f);

void graph_free(graph *g);

// Functions for constructing a graph

void graph_sort_edges(graph *g);

void graph_add_vertex(graph *g, long long w);

void graph_add_edge(graph *g, int u, int v);

// After construction, use these to maintain sorted neighborhoods

void graph_remove_edge(graph *g, int u, int v);

void graph_insert_edge(graph *g, int u, int v);

void graph_deactivate_vertex(graph *g, int u);

void graph_activate_vertex(graph *g, int u);

void graph_deactivate_neighborhood(graph *g, int u);

void graph_activate_neighborhood(graph *g, int u);