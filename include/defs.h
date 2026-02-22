#pragma once

#include <signal.h>

#define ALLOC_N_INIT (1 << 10)
#define ALLOC_DEGREE_INIT (1 << 4)
#define N_BUFFERS 4
#define MAX_DOMINATION (1 << 7)
#define MAX_SIMPLICIAL_VERTEX (1 << 7)
#define MAX_TWIN (1 << 7)
#define MAX_UNCONFINED (1 << 7)
#define MAX_STRUCTION_NODES (1 << 5)
#define MAX_STRUCTION_DEGREE (1 << 4)

extern volatile sig_atomic_t keep_running;