#include "simplicial_vertex_with_weight_transfer.h"
#include "algorithms.h"

#include <assert.h>
#include <stdlib.h>

int simplicial_vertex_with_weight_transfer_reduce_graph(graph *g, node_id u, node_weight *offset,
                                                        buffers *b, change_list *c, reconstruction_data *d)
{
    assert(g->A[u]);

    if (g->D[u] > MAX_SIMPLICIAL_VERTEX)
        return 0;

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        // if d[v] is smaller than d[u], then N(u) can not be clique
        // if v and u have same degree, both are potentially simplicial then,
        // we want to check only for the highest weight vertex
        if (g->D[v] < g->D[u])
            return 0;
    }

    node_id max_weight_v_simplicial = u;
    int higher_weight_vertex = 0;

    for (node_id i = 0; i < g->D[u]; i++)
    {
        node_id v = g->V[u][i];

        if (g->W[max_weight_v_simplicial] < g->W[v])
        {
            if (g->D[v] == g->D[u])
                max_weight_v_simplicial = v;
            else
                higher_weight_vertex = 1;
        }

        // check if neighborhood is a clique via is_subset
        if (!set_is_subset_except_one(g->V[u], g->D[u], g->V[v], g->D[v], v))
            return 0;
    }

    *offset = g->W[max_weight_v_simplicial];
    d->u = max_weight_v_simplicial;
    d->n = g->l;
    d->data = NULL;

    if (!higher_weight_vertex) // simplicial vertex reduction
    {
        d->z = 0;
        graph_deactivate_neighborhood(g, max_weight_v_simplicial);
    }
    else // simplicial weight transfer
    {
        d->z = 1;
        d->x = 0;
        graph_deactivate_vertex(g, max_weight_v_simplicial);
        node_id *remaining_vertices = malloc(sizeof(node_id) * g->D[u]);
        for (node_id i = 0; i < g->D[max_weight_v_simplicial]; i++)
        {
            node_id v = g->V[max_weight_v_simplicial][i];

            if (g->W[v] <= g->W[max_weight_v_simplicial])
            {
                graph_deactivate_vertex(g, v);
            }
            else
            {
                graph_change_vertex_weight(g, v, g->W[v] - g->W[max_weight_v_simplicial]);
                remaining_vertices[d->x++] = v;
            }
        }
        d->data = (void *)remaining_vertices;
    }

    reduction_data_queue_distance_two(g, max_weight_v_simplicial, c);
    return 1;
}

void simplicial_vertex_with_weight_transfer_restore_graph(graph *g, reconstruction_data *d)
{
    assert(!g->A[d->u]);

    graph_undo_changes(g, d->n);
}

void simplicial_vertex_with_weight_transfer_reconstruct_solution(int *I, reconstruction_data *d)
{
    if (d->z == 0)
    {
        I[d->u] = 1;
    }
    else
    {
        node_id *remaining_vertices = (node_id *)d->data;
        for (node_id i = 0; i < d->x; i++)
        {
            node_id v = remaining_vertices[i];
            if (I[v])
                return;
        }

        // if none of the remaining vertices was in the solution, we pick u
        I[d->u] = 1;
    }
}

void simplicial_vertex_with_weight_transfer_clean(reconstruction_data *d)
{
    node_id *remaining_vertices = (node_id *)d->data;
    free(remaining_vertices);
}