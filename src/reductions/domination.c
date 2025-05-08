#include "domination.h"
#include "algorithms.h"

#include <assert.h>

// int domination_reduce_graph(graph *g, node_id u, node_weight *offset,
//                             buffers *b, change_list *c, reconstruction_data *d)
// {
//     assert(g->A[u]);

//     for (node_id i = 0; i < g->D[u]; i++)
//     {
//         int v = g->V[u][i];

//         if (g->W[v] >= g->W[u] && g->D[v] <= g->D[u] &&
//             set_is_subset_except_one(g->V[v], g->D[v], g->V[u], g->D[u], u))
//         {
//             *offset = 0;
//             d->u = u;
//             graph_deactivate_vertex(g, u);

//             reduction_data_queue_distance_one(g, u, c);

//             return 1;
//         }
//     }

//     return 0;
// }

int domination_reduce_graph(graph *g, node_id u, node_weight *offset,
                            buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    node_id md = -1;
    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        if (md < 0 || g->D[v] < g->D[md])
            md = v;
    }

    if (md < 0 || g->D[md] > MAX_DOMINATION)
        return 0;

    if (g->W[md] <= g->W[u] && g->D[md] >= g->D[u] &&
        set_is_subset_except_one(g->V[u], g->D[u], g->V[md], g->D[md], md))
    {
        *offset = 0;
        d->u = md;
        graph_deactivate_vertex(g, md);

        reduction_data_queue_distance_one(g, md, c);

        return 1;
    }

    for (node_id i = 0; i < g->D[md]; i++)
    {
        int v = g->V[md][i];

        if (v != u && g->W[v] <= g->W[u] && g->D[v] >= g->D[u] && graph_is_neighbor(g, u, v) &&
            set_is_subset_except_one(g->V[u], g->D[u], g->V[v], g->D[v], v))
        {
            *offset = 0;
            d->u = v;
            graph_deactivate_vertex(g, v);

            reduction_data_queue_distance_one(g, v, c);

            return 1;
        }
    }

    return 0;
}

void domination_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_activate_vertex(g, d->u);
}

void domination_reconstruct_solution(int *I, reconstruction_data *d)
{
    I[d->u] = 0;
}

void domination_clean(reconstruction_data *d)
{
}