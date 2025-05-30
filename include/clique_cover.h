#pragma once

#include <graph.h>

/*
    Greedy clique cover heuristics based on graph coloring.

    To avoid dynamic allocation, all internal buffers are
    passed as arguments to the functinos. The only time
    the contents of these buffers matter is for the
    "from_priority" function. There, the P array specifies
    the ordering to use for the clique generation.
*/

void clique_cover_first_fit(graph *g, int *C, int *P, int *T, node_id *O);

void clique_cover_largest_degree_first(graph *g, int *C, int *P, int *T, node_id *O);

void clique_cover_from_priority(graph *g, int *C, int *P, int *T, node_id *O);

int clique_cover_validate(graph *g, const int *C, int *T);

node_weight clique_cover_upper_bound(graph *g, const int *C, node_id *M);