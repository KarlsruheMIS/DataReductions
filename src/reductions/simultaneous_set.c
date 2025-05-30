// #include "simultaneous_set.h"
// #include "algorithms.h"

// #include "reducer.h"

// #include "degree_zero.h"
// #include "degree_one.h"
// #include "neighborhood_removal.h"
// #include "triangle.h"
// #include "v_shape.h"
// #include "domination.h"
// #include "twin.h"
// #include "simplicial_vertex_with_weight_transfer.h"
// #include "weighted_funnel.h"
// #include "unconfined.h"
// #include "extended_domination.h"

// #include <assert.h>
// #include <stdlib.h>

// int simultaneous_set_test(graph *g, node_id u, int *fs1, int *fs2, node_id *FM, int k)
// {
//     graph *sg = graph_init();
//     for (node_id i = 0; i < g->D[u]; i++)
//     {
//         node_id v = g->V[u][i];
//         if (fs1[v] == k)
//             continue;

//         fs2[v] = k;
//         graph_add_vertex(sg, g->W[v]);
//         FM[v] = sg->n - 1;
//     }

//     for (node_id i = 0; i < g->D[u]; i++)
//     {
//         node_id v = g->V[u][i];
//         if (fs1[v] == k)
//             continue;

//         for (node_id j = 0; j < g->D[v]; j++)
//         {
//             node_id w = g->V[v][j];
//             if (fs2[w] == k && v < w)
//                 graph_add_edge(sg, FM[v], FM[w]);
//         }
//     }

//     for (node_id i = 0; i < g->D[u]; i++)
//     {
//         node_id v = g->V[u][i];
//         if (fs1[v] == k)
//             continue;

//         fs2[v] = k - 1;
//     }

//     graph_sort_edges(sg);

//     reducer *r = reducer_init(sg, 11,
//                               degree_zero,
//                               degree_one,
//                               neighborhood_removal,
//                               triangle,
//                               v_shape,
//                               domination,
//                               twin,
//                               simplicial_vertex_with_weight_transfer,
//                               weighted_funnel,
//                               unconfined,
//                               extended_domination);

//     reduction_log *l = reducer_reduce(r, sg, 60.0);

//     int res = sg->nr == 0 && l->offset <= g->W[u];

//     reducer_free_reduction_log(l);
//     reducer_free(r);
//     graph_free(sg);

//     return res;
// }

// int simultaneous_set_test_reverse(graph *g, node_id u, node_id v, int *fs1, int *fs2, node_id *FM, int k)
// {
//     for (node_id i = 0; i < g->D[v]; i++)
//     {
//         node_id w = g->V[v][i];
//         fs1[w] = k;
//     }

//     int res = simultaneous_set_test(g, u, fs1, fs2, FM, k);

//     for (node_id i = 0; i < g->D[v]; i++)
//     {
//         node_id w = g->V[v][i];
//         fs1[w] = k - 1;
//     }

//     return res;
// }

// int simultaneous_set_reduce_graph(graph *g, node_id u, node_weight *offset,
//                                   buffers *b, change_list *c, reconstruction_data *d)
// {
//     assert(g->A[u] && N_BUFFERS >= 3);

//     int *fs1 = b->fast_sets[0], *fs2 = b->fast_sets[1], *fs3 = b->fast_sets[2];
//     node_id *FM = b->buffers[0];
//     int k = b->t;

//     // Assume vertex is in the solution
//     fs1[u] = k;
//     for (node_id i = 0; i < g->D[u]; i++)
//     {
//         node_id v = g->V[u][i];
//         fs1[v] = k;
//     }

//     for (node_id i = 0; i < g->D[u]; i++)
//     {
//         node_id v = g->V[u][i];
//         for (node_id j = 0; j < g->D[v]; j++)
//         {
//             node_id w = g->V[v][j];
//             if (fs1[w] == k)
//                 continue;

//             if (simultaneous_set_test(g, w, fs1, fs2, FM, k) &&
//                 simultaneous_set_test_reverse(g, u, w, fs2, fs3, FM, k))
//             {
//                 *offset = 0;

//                 d->u = u;
//                 d->v = w;

//                 graph_deactivate_vertex(g, w);
//                 g->W[u] += g->W[w];

//                 d->n = 0;
//                 node_id *added_edges = malloc(sizeof(node_id) * g->D[w]);
//                 for (node_id l = 0; l < g->D[w]; l++)
//                 {
//                     node_id x = g->V[w][l];
//                     if (fs3[x] < k && !graph_is_neighbor(g, u, x))
//                     {
//                         fs3[x] = k;
//                         added_edges[d->n++] = x;
//                         graph_insert_edge(g, u, x);
//                     }
//                 }
//                 d->data = (void *)added_edges;

//                 reduction_data_queue_distance_one(g, u, c);

//                 return 1;
//             }
//         }
//     }

//     return 0;
// }

// void simultaneous_set_restore_graph(graph *g, reconstruction_data *d)
// {
//     assert(!g->A[d->v]);

//     node_id *added_edges = (node_id *)d->data;
//     for (node_id i = 0; i < d->n; i++)
//     {
//         graph_remove_edge(g, d->u, added_edges[i]);
//     }

//     graph_activate_vertex(g, d->v);

//     g->W[d->u] -= g->W[d->v];
// }

// void simultaneous_set_reconstruct_solution(int *I, reconstruction_data *d)
// {
//     I[d->v] = I[d->u];
// }

// void simultaneous_set_clean(reconstruction_data *d)
// {
//     node_id *added_edges = (node_id *)d->data;
//     free(added_edges);
// }