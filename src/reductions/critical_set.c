#include "critical_set.h"
#include "algorithms.h"

#include <limits.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>

// For clarity
typedef node_id edge_id;
typedef node_weight edge_weight;
typedef node_weight flow_t;

#define INFINITE_FLOW LLONG_MAX

typedef struct {
    // Number of vertices in the flow graph.
    node_id n;
    // Source and sink indices in the flow graph.
    node_id source, sink;
    // The total capacity of all edges leaving the source.
    flow_t outgoing_source_cap;
    // Number of edges in the flow graph.
    edge_id m;
    // For edge e = (u,v), heads[e] == v.
    node_id *heads;
    // For edge e = (u,v), rev_idxs[e] = (v,u).
    edge_id *rev_idxs;
    // Stores the residual capacity for each edge.
    flow_t *residuals;
    // For vertex v, its outgoing edges are [edge_idxs[v], edge_idxs[v+1]).
    edge_id *edge_idxs;

    // Used to store in-progress edge indices in a few places.
    edge_id *progress;
    // Vertex queue used for BFS.
    node_id *queue;
    node_id q_start, q_end;
    // A distance map used in the dinitz BFS and DFS.
    node_id *ranks;
} dinitz_fg;

typedef struct {
    // For the i-th active node with index j, forward[j] == i.
    node_id *forward;
    // For the i-th active node with index j, reverse[i] == j.
    node_id *reverse;
    // Total number of nodes. equal to the length of forward.
    node_id total;
    // Active number of nodes. equal to the length of reverse.
    node_id active;
} node_map;

static void add_edges(dinitz_fg *fg, node_id *progress, node_id u, node_id v,
                      flow_t f) {
    edge_id e = progress[u]++, e_r = progress[v]++;

    fg->heads[e] = v;
    fg->heads[e_r] = u;

    fg->residuals[e] = f;
    fg->residuals[e_r] = 0;

    fg->rev_idxs[e] = e_r;
    fg->rev_idxs[e_r] = e;
}

static void build_fg(dinitz_fg *fg, const node_map *nm, const graph *g) {
    fg->n = nm->active * 2 + 2;
    fg->edge_idxs = calloc(fg->n + 1, sizeof(edge_id));
    fg->progress = malloc(fg->n * sizeof(edge_id));
    fg->queue = malloc(fg->n * sizeof(edge_id));
    fg->ranks = malloc(fg->n * sizeof(edge_id));

    edge_id *counters = &fg->edge_idxs[1];
    fg->source = fg->n - 2;
    fg->sink = fg->n - 1;
    for (node_id u = 0; u < nm->active; u++) {
        node_id u_orig = nm->reverse[u];
        counters[fg->source]++;
        counters[u]++;

        counters[fg->sink]++;
        counters[u + nm->active]++;

        for (edge_id e = 0; e < g->D[u_orig]; e++) {
            node_id v_orig = g->V[u_orig][e];
            if (g->A[v_orig]) {
                counters[u]++;
                counters[nm->forward[v_orig] + nm->active]++;
            }
        }
    }

    for (node_id i = 1; i <= fg->n; i++) {
        fg->edge_idxs[i] += fg->edge_idxs[i - 1];
    }

    fg->m = fg->edge_idxs[fg->n];
    fg->heads = malloc(fg->m * sizeof(node_id));
    fg->rev_idxs = malloc(fg->m * sizeof(edge_id));
    fg->residuals = malloc(fg->m * sizeof(flow_t));

    memcpy(fg->progress, fg->edge_idxs, fg->n * sizeof(edge_id));

    fg->outgoing_source_cap = 0;

    for (node_id u = 0; u < nm->active; u++) {
        node_id u_orig = nm->reverse[u];
        flow_t weight = g->W[u_orig];
        fg->outgoing_source_cap += weight;

        add_edges(fg, fg->progress, fg->source, u, weight);
        add_edges(fg, fg->progress, u + nm->active, fg->sink, weight);

        for (edge_id e = 0; e < g->D[u_orig]; e++) {
            node_id v_orig = g->V[u_orig][e];
            if (g->A[v_orig]) {
                add_edges(fg, fg->progress, u, nm->forward[v_orig] + nm->active,
                          weight);
            }
        }
    }
}

static void free_fg(dinitz_fg *fg) {
    free(fg->heads);
    free(fg->rev_idxs);
    free(fg->residuals);
    free(fg->edge_idxs);
    free(fg->progress);
    free(fg->queue);
    free(fg->ranks);
}

static bool compute_ranks(dinitz_fg *fg) {
    memset(fg->ranks, 0, sizeof(node_id) * fg->n);
    fg->q_start = 0;
    fg->q_end = 1;
    fg->queue[0] = fg->source;
    fg->ranks[fg->source] = 1;

    while (fg->q_start != fg->q_end) {
        node_id x = fg->queue[fg->q_start++];
        if (x == fg->sink)
            return true;

        for (edge_id e = fg->edge_idxs[x]; e < fg->edge_idxs[x + 1]; e++) {
            node_id y = fg->heads[e];
            if (fg->residuals[e] > 0 && fg->ranks[y] == 0) {
                fg->ranks[y] = fg->ranks[x] + 1;
                fg->queue[fg->q_end++] = y;
            }
        }
    }

    return false;
}

static flow_t augment(dinitz_fg *fg, node_id x, flow_t flow) {
    if (x == fg->sink) {
        return flow;
    }

    while (fg->progress[x] != fg->edge_idxs[x + 1]) {
        edge_id e = fg->progress[x];
        node_id y = fg->heads[e];

        if (fg->residuals[e] > 0 && fg->ranks[y] == fg->ranks[x] + 1) {
            flow_t eps = augment(
                fg, y, fg->residuals[e] < flow ? fg->residuals[e] : flow);
            if (eps > 0) {
                fg->residuals[e] -= eps;
                fg->residuals[fg->rev_idxs[e]] += eps;
                return eps;
            }
        }

        fg->progress[x]++;
    }

    return 0;
}

static flow_t solve_max_flow(dinitz_fg *fg) {
    flow_t result = 0;

    while (compute_ranks(fg)) {
        memcpy(fg->progress, fg->edge_idxs, fg->n * sizeof(edge_id));
        flow_t aug;
        while ((aug = augment(fg, fg->source, INFINITE_FLOW)) != 0) {
            result += aug;
        }
    }

    return result;
}

// `red` should point to an array with enough capacity to hold all vertices
// that should be reduced (e.g. just `g->n`). After the function returns,
// `red[0..n_red]` will contain the critical set of the graph.
static node_id nodes_to_reduce(const dinitz_fg *fg, const node_map *nm,
                               const graph *g, node_id *red) {
    // Dinitz calculated the ranks just before returning, thus the rank map is
    // currently up to date. Therefore, a node in the residual graph is
    // reachable iff it has a nonzero rank.
    node_id n_red = 0;
    for (node_id u = 0; u < nm->active; u++) {
        if (fg->ranks[u]) {
            red[n_red++] = nm->reverse[u];
        }
    }

    return n_red;
}

void build_node_map(node_map *map, const graph *g) {
    node_id num_active = 0;
    for (node_id i = 0; i < g->n; i++) {
        num_active += (g->A[i] != 0);
    }

    map->total = g->n;
    map->active = num_active;
    map->forward = malloc(g->n * sizeof(node_id));
    map->reverse = malloc(num_active * sizeof(node_id));

    node_id progress = 0;
    for (node_id i = 0; i < g->n; i++) {
        if (g->A[i]) {
            map->forward[i] = progress;
            map->reverse[progress] = i;
            progress++;
        }
    }
}

static void free_node_map(node_map *nm) {
    free(nm->forward);
    free(nm->reverse);
}

int critical_set_reduce_graph(graph *g, node_id u, node_weight *offset,
                              buffers *b, change_list *c,
                              reconstruction_data *d) {
    node_map nm;
    build_node_map(&nm, g);
    dinitz_fg fg;
    build_fg(&fg, &nm, g);
    flow_t aug = solve_max_flow(&fg);

    if (aug < fg.outgoing_source_cap) {
        node_id *red = malloc(g->n * sizeof(node_id));
        node_id n_red = nodes_to_reduce(&fg, &nm, g, red);

        d->n = g->l;
        d->x = n_red;
        *offset = 0;
        d->data = (void *)realloc(red, n_red * sizeof(node_id));

        for (node_id i = 0; i < n_red; i++) {
            node_id u = red[i];
            *offset += g->W[u];
            graph_deactivate_neighborhood(g, u);
            reduction_data_queue_distance_two(g, u, c);
        }
    }

    free_fg(&fg);
    free_node_map(&nm);

    return aug < fg.outgoing_source_cap;
}

void critical_set_restore_graph(graph *g, reconstruction_data *d) {
    graph_undo_changes(g, d->n);
}

void critical_set_reconstruct_solution(int *I, reconstruction_data *d) {
    node_id *red = (node_id *)d->data;
    node_id n_red = d->x;

    for (node_id i = 0; i < n_red; i++) {
        node_id u = red[i];
        I[u] = 1;
    }
}

void critical_set_clean(reconstruction_data *d) { free(d->data); }