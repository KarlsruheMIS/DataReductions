#pragma once

#ifndef __NODE_TYPES__
#define __NODE_TYPES__

#define node_id int
#define node_weight long long

#endif

#ifndef __GRAPH__
#define __GRAPH__

/*
    Dynamic graph structure used for reductions
*/

typedef struct
{
    long long n, m, nr; // Number of vertices, edges, and remaining vertices
    node_id **V, *D;    // Adjacency lists and degrees
    node_weight *W;     // Vertex weights
    int *A;             // Active flags

    long long _a;  // Number of vertices allocated memory for
    long long *_A; // Allocated memory per neighborhood
} graph;

graph *graph_init();

void graph_add_vertex(graph *g, node_weight w);

void graph_add_edge(graph *g, node_id u, node_id v);

void graph_free(graph *g);

#endif

/*
    Main functions for reduction process
*/

void *mwis_reduction_reduce_graph(graph *g, double tl);

void *mwis_reduction_run_struction(graph *g, double tl);

int *mwis_reduction_lift_solution(int *rI, void *reduction_data);

void mwis_reduction_restore_graph(graph *rg, void *reduction_data);

void mwis_reduction_free(void *reduction_data);

long long mwis_reduction_get_offset(void *reduction_data);