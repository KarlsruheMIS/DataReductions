#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#define node_id int
#define node_weight long long

    /**
     * @brief Enum representing the different reduction rules available.
     * These values are used to specify which rules to apply.
     */
    typedef enum
    {
        DEGREE_ZERO = 1000, // values outside ASCII range
        DEGREE_ONE,
        NEIGHBORHOOD_REMOVAL,
        TRIANGLE,
        V_SHAPE,
        DOMINATION,
        TWIN,
        SIMPLICIAL_VERTEX,
        SIMPLICIAL_WEIGHT_TRANSFER,
        FUNNEL,
        UNCONFINED,
        EXTENDED_DOMINATION,
        CRITICAL_SET,
        STRUCTION_SPARSE,
        STRUCTION_DENSE,
        REDUCTION_FORCE_32BIT = 0x7FFFFFFF
    } reduction_t;

    /**
     * @brief A graph structure for the MWIS reduction process.
     * This struct holds all information about the graph and the reduction process.
     * It is designed to be an opaque type for the library user.
     */
    typedef struct
    {
        long long n, m, nr; // Number of vertices, edges, and remaining active vertices
        node_id **V, *D;    // Adjacency lists and degrees
        node_weight *W;     // Vertex weights
        int *A;             // Active flags
        long long offset;   // Solution offset

        void *internal_graph;         // Internal data, do not modify
        void *internal_reduction_log; // Internal data, do not modify
    } mwis_reduction_graph;

    /**
     * @brief Reads a graph from a file in METIS format.
     * @param Path The path to the graph file.
     * @return A pointer to the newly created mwis_reduction_graph.
     */
    mwis_reduction_graph *mwis_reduction_graph_read_from_file(const char *Path);

    /**
     * @brief Initializes an empty graph object.
     * @return A pointer to the newly created mwis_reduction_graph.
     */
    mwis_reduction_graph *mwis_reduction_graph_init();

    /**
     * @brief Adds a vertex to the graph with ID equal to g->n prior to adding it.
     * @param g The graph to modify.
     * @param w The weight of the new vertex.
     */
    void mwis_reduction_graph_add_vertex(mwis_reduction_graph *g, node_weight w);

    /**
     * @brief Adds an edge between two vertices. Assumes both endpoints exist in the graph.
     * @param g The graph to modify.
     * @param u The first vertex.
     * @param v The second vertex.
     */
    void mwis_reduction_graph_add_edge(mwis_reduction_graph *g, node_id u, node_id v);

    /**
     * @brief Frees all memory associated with the graph object.
     * @param g The graph to free.
     */
    void mwis_reduction_graph_free(mwis_reduction_graph *g);

    /**
     * @brief Applies a series of reduction rules to the graph.
     * The graph is modified in place.
     * @param g The graph to reduce.
     * @param n_rules The number of rules to apply.
     * @param Rules An array of reduction rules to apply.
     */
    void mwis_reduction_reduce_graph(mwis_reduction_graph *g, int n_rules, reduction_t *Rules);

    /**
     * @brief Runs the struction algorithm on the graph, accepting changes that reduce |V| and |E|.
     * Struction is a technique to handle local solutions and can further reduce the graph.
     * This "sparse" version is more restrictive and faster.
     * @param g The graph to apply struction on.
     * @param n_rules The number of rules to use within the struction algorithm.
     * @param Rules An array of reduction rules to use.
     */
    void mwis_reduction_run_struction_sparse(mwis_reduction_graph *g, int n_rules, reduction_t *Rules);

    /**
     * @brief Runs the struction algorithm on the graph, accepting changes that reduce |V|.
     * This "dense" version is less restrictive than the sparse one and can achieve better reductions,
     * but may be slower and lead to higher number of edges.
     * @param g The graph to apply struction on.
     * @param n_rules The number of rules to use within the struction algorithm.
     * @param Rules An array of reduction rules to use.
     */
    void mwis_reduction_run_struction_dense(mwis_reduction_graph *g, int n_rules, reduction_t *Rules);

    /**
     * @brief Restores the graph to its original state before any reductions were applied.
     * @param g The graph to restore.
     */
    void mwis_reduction_restore_graph(mwis_reduction_graph *g);

    /**
     * @brief Lifts a solution from the reduced graph to the original graph.
     * After applying reductions, you can solve the smaller (kernel) graph and then use this
     * function to obtain the solution for the original graph.
     * @param g The graph object.
     * @param I An array of integers of size g->n. It should represent the solution on the reduced graph (1 if vertex is in IS, 0 otherwise).
     *          The array will be modified to represent the solution on the original graph.
     */
    void mwis_reduction_lift_solution(mwis_reduction_graph *g, int *I);

    /**
     * @brief Requests the reduction process to stop gracefully.
     * This function can be called from a signal handler or another thread to interrupt
     * a long-running reduction process.
     */
    void mwis_reduction_request_stop();

#ifdef __cplusplus
}
#endif