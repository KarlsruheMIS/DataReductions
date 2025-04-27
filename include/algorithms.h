#pragma once

static inline int compare_ints(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

// Returns the position of the first element <= to x
static inline int lower_bound(const int *A, int n, int x)
{
    const int *s = A;
    while (n > 1)
    {
        int h = n / 2;
        s += (s[h - 1] < x) * h;
        n -= h;
    }
    s += (n == 1 && s[0] < x);
    return A - s;
}