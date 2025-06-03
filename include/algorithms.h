#pragma once

#include "defs.h"

#include <time.h>

static inline double get_wtime()
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (double)tp.tv_sec + ((double)tp.tv_nsec / 1e9);
}

static inline int compare_ids(const void *a, const void *b)
{
    return (*(node_id *)a - *(node_id *)b);
}

// Returns the position of the first element >= to x
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
    return s - A;
}

// Test if A and B are equal
static inline int set_is_equal(const node_id *A, node_id a, const node_id *B, node_id b)
{
    if (b != a)
        return 0;

    for (node_id i = 0; i < a; i++)
    {
        if (A[i] != B[i])
            return 0;
    }

    return 1;
}

// Test if A is a subset of B
static inline int set_is_subset(const node_id *A, node_id a, const node_id *B, node_id b)
{
    if (b < a)
        return 0;

    node_id i = 0, j = 0;
    while (i < a && j < b && (a - i <= b - j))
    {
        if (A[i] == B[j])
        {
            i++;
            j++;
        }
        else if (A[i] > B[j])
        {
            j++;
        }
        else
        {
            return 0;
        }
    }

    return i == a;
}

// Test if A is a subset of B, ignoring x from A
static inline int set_is_subset_except_one(const node_id *A, node_id a, const node_id *B, node_id b, node_id x)
{
    if (b < a - 1)
        return 0;

    node_id i = 0, j = 0;
    while (i < a && j < b)
    {
        if (A[i] == B[j])
        {
            i++;
            j++;
        }
        else if (A[i] > B[j])
        {
            j++;
        }
        else if (A[i] == x)
        {
            i++;
        }
        else
        {
            return 0;
        }
    }

    if (i < a && A[i] == x)
        i++;

    return i == a;
}

// Test if A is a subset of B, ignoring x and y from A
static inline int set_is_subset_except_two(const node_id *A, node_id a, const node_id *B, node_id b, node_id x, node_id y)
{
    if (b < a - 2)
        return 0;

    node_id i = 0, j = 0;
    while (i < a && j < b)
    {
        if (A[i] == B[j])
        {
            i++;
            j++;
        }
        else if (A[i] > B[j])
        {
            j++;
        }
        else if (A[i] == x || A[i] == y)
        {
            i++;
        }
        else
        {
            return 0;
        }
    }

    while (i < a && (A[i] == x || A[i] == y))
        i++;

    return i == a;
}

// Computes A \ (B U {x}) and stores the result in C, returns the size of C
static inline int set_minus_except_one(const node_id *A, node_id a, const node_id *B, node_id b, node_id *C, node_id x)
{
    node_id n = 0;
    node_id i = 0, j = 0;
    while (i < a && j < b)
    {
        if (A[i] == B[j])
        {
            i++;
            j++;
        }
        else if (A[i] > B[j])
        {
            j++;
        }
        else
        {
            if (A[i] != x)
                C[n++] = A[i];
            i++;
        }
    }

    while (i < a)
    {
        if (A[i] != x)
            C[n++] = A[i];
        i++;
    }

    return n;
}