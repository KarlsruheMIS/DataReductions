#include "reduction.h"

#include <stdlib.h>

void reduction_data_init(graph *g, buffers **b, change_list **c)
{
    *b = malloc(sizeof(buffers));

    (*b)->t = 1;
    (*b)->_a = g->_a;
    (*b)->buffers = malloc(sizeof(node_id *) * N_BUFFERS);
    (*b)->fast_sets = malloc(sizeof(int *) * N_BUFFERS);

    for (int i = 0; i < N_BUFFERS; i++)
    {
        (*b)->buffers[i] = malloc(sizeof(node_id) * (*b)->_a);
        (*b)->fast_sets[i] = malloc(sizeof(int) * (*b)->_a);

        for (long long j = 0; j < (*b)->_a; j++)
        {
            (*b)->fast_sets[i][j] = 0;
        }
    }

    *c = malloc(sizeof(change_list));

    (*c)->_a = g->_a;
    (*c)->n = 0;
    (*c)->V = malloc(sizeof(node_id) * (*c)->_a);
}

void reduction_data_reset_fast_sets(buffers *b)
{
    b->t = 0;
    for (int i = 0; i < N_BUFFERS; i++)
        for (node_id j = 0; j < b->_a; j++)
            b->fast_sets[i][j] = 0;
}

void reduction_data_increase(buffers *b, change_list *c)
{
    b->_a *= 2;
    b->t = 1;

    for (int i = 0; i < N_BUFFERS; i++)
    {
        b->buffers[i] = realloc(b->buffers[i], sizeof(node_id) * b->_a);
        b->fast_sets[i] = realloc(b->fast_sets[i], sizeof(int) * b->_a);

        for (long long j = 0; j < b->_a; j++)
        {
            b->fast_sets[i][j] = 0;
        }
    }

    c->_a *= 2;
    c->n = 0;
    c->V = realloc(c->V, sizeof(node_id) * c->_a);
}

void reduction_data_free(buffers *b, change_list *c)
{
    for (int i = 0; i < N_BUFFERS; i++)
    {
        free(b->buffers[i]);
        free(b->fast_sets[i]);
    }
    free(b->buffers);
    free(b->fast_sets);
    free(b);

    free(c->V);
    free(c);
}