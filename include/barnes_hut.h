#pragma once

#include "graph.h"

#define INNER_WIDTH 8
#define GRID_WIDTH 16384

typedef struct
{
    // --- Quad-Tree structure ---

    int m;            // Number of cells in the quad-tree
    int l;            // Height of the quad-trees
    float *CX, *CY;   // Center position in each cell
    float *Mass;      // Total mass in each cell
    float *R;         // Average vertex radius in each cell
    int *C;           // Number of vertices in each cell
    int *S;           // Cell width
    int **Queue;      // Queues for vertex insertion
    int **Queue_mark; // Marks for vertices in the queue

    // --- Vertex forces ---

    int n;          // Number of vertices
    float *X, *Y;   // Vertex positions
    float *fX, *fY; // Vertex forces
    float *vX, *vY; // Vertex velocities
    int *Tabu;      // Tabu boolen vector, tabu vertices stay fixed

    // --- Parameters ---

    float rest_l;    // Prefered link length
    float k_spring;  // Spring force
    float k_repel;   // Repel force
    float k_gravity; // Gravity force
    float theta;     // Barnes Hut distance parameter, O(n^2) at 0

} barnes_hut;

barnes_hut *barnes_hut_init(graph *g);

void barnes_hut_free(barnes_hut *bh);

void barnes_hut_step(barnes_hut *bh, graph *g);