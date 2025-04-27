#pragma once
#include <stdio.h>

typedef struct
{
    int n, m, a;
    int *D, *S, *A;
    int **V;
    long long *W;
} dynamic_graph;

dynamic_graph *dynamic_graph_parse(FILE *f);

void dynamic_graph_store(FILE *f, dynamic_graph *g);

void dynamic_graph_free(dynamic_graph *g);

int dynamic_graph_validate(dynamic_graph *g);

dynamic_graph *dynamic_graph_subgraph(dynamic_graph *g, int *mask, int *reverse_mapping);