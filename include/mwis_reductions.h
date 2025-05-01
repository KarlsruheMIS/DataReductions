#pragma once

#define node_id int
#define node_weight long long

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

void graph_add_vertex(graph *g, node_weight w);

void graph_add_edge(graph *g, node_id u, node_id v);

void graph_free(graph *g);


/*
    Main functions for reduction process
*/

void *mwis_reduction_reduce_graph(graph *g);

int *mwis_reduction_lift_solution(int *rI, void *reduction_data);

void mwis_reduction_restore_graph(graph *rg, void *reduction_data);

void mwis_reduction_free(void *reduction_data);