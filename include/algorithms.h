#pragma once

#include "defs.h"

static inline int compare_ids(const void *a, const void *b)
{
    return (*(node_id *)a - *(node_id *)b);
}

// Returns the position of the first element <= to x
static inline node_id lower_bound(const node_id *A, node_id n, node_id x)
{
    const node_id *s = A;
    while (n > 1)
    {
        node_id h = n / 2;
        s += (s[h - 1] < x) * h;
        n -= h;
    }
    s += (n == 1 && s[0] < x);
    return A - s;
}