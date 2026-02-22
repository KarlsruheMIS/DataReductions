#include <iostream>
#include <vector>

#include "mwis_reductions.h"

/*
    This is an example showing how to reduce a graph using our datareductio
    library. You can either parse a graph directly from file or build it
    in memory using the mwis_reduction_graph_add_vertex / add_edge functions.
*/

// This function validates that the solution is independet and returns its weight
long long validate_mwis(mwis_reduction_graph *g, const std::vector<int> &I)
{
    long long cost = 0;
    for (int u = 0; u < g->n; u++)
    {
        if (!g->A[u])
            return -1;

        if (!I[u])
            continue;

        cost += g->W[u];
        for (int i = 0; i < g->D[u]; i++)
        {
            int v = g->V[u][i];
            if (!g->A[v] || I[v])
                return -1;
        }
    }
    return cost;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " [Path to graph]" << std::endl;
        return 0;
    }

    mwis_reduction_graph *g = mwis_reduction_graph_read_from_file(argv[1]);

    std::cout << "Parsed graph V=" << g->nr << " E=" << g->m << std::endl;

    std::vector<reduction_t> Rules_reduce = {DEGREE_ZERO,
                                             DEGREE_ONE,
                                             NEIGHBORHOOD_REMOVAL,
                                             DOMINATION,
                                             UNCONFINED,
                                             CRITICAL_SET};

    mwis_reduction_reduce_graph(g, Rules_reduce.size(), Rules_reduce.data());

    std::cout << "After reductions V=" << g->nr << " E=" << g->m << std::endl;

    // Avoid heavy global rules when running struction
    std::vector<reduction_t> Rules_struction = {DEGREE_ZERO,
                                                DEGREE_ONE,
                                                NEIGHBORHOOD_REMOVAL,
                                                DOMINATION,
                                                UNCONFINED};

    mwis_reduction_run_struction_sparse(g, Rules_struction.size(), Rules_struction.data());

    std::cout << "After struction sparse V=" << g->nr << " " << g->m << std::endl;

    mwis_reduction_run_struction_dense(g, Rules_struction.size(), Rules_struction.data());

    std::cout << "After struction dense V=" << g->nr << " " << g->m << std::endl;

    long long cost = g->offset;

    /*
        Use a list of integers with 0/1 of size g->n to represent the solution on the reduced graph.
        This is possible larger than the number of active vertices, but after calling "lift_solution"
        the part 0...N-1 will contain the solution on the original unreduced graph.

        You can at any point know the size of the solution by combining g->offset with the size of
        your solution on the remaining vertices.
    */
    std::vector<int> I(g->n, 0);

    std::cout << "Lifting solution" << std::endl;

    mwis_reduction_lift_solution(g, I.data());

    std::cout << "Restoring graph" << std::endl;

    mwis_reduction_restore_graph(g);

    long long valid_cost = validate_mwis(g, I);

    std::cout << "Restored graph V=" << g->nr << " E=" << g->m << std::endl;

    std::cout << "Reported solution size "  << cost << ", verified solution size " << valid_cost << std::endl;

    mwis_reduction_graph_free(g);

    return 0;
}