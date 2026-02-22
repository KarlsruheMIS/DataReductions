#include "graph.h"
#include "reducer.h"
#include "algorithms.h"
#include "struction_runner.h"

#include "degree_zero.h"
#include "degree_one.h"
#include "neighborhood_removal.h"
#include "triangle.h"
#include "v_shape.h"
#include "domination.h"
#include "twin.h"
#include "simplicial_vertex.h"
#include "simplicial_vertex_with_weight_transfer.h"
#include "weighted_funnel.h"
#include "unconfined.h"
#include "extended_domination.h"
#include "critical_set.h"
#include "struction.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <getopt.h>

volatile sig_atomic_t keep_running = 1;

void handle_stop(int sig)
{
    keep_running = 0;
}

void print_help(const char *prog)
{
    printf(
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -h, --help               Show this help message\n"
        "  -g, --graph              Path to the input graph in METIS format\n"
        "  -s, --solution           Path to store the solution in the event of an empty kernel\n"
        "  -v, --verbose            Verbose mode, prints continous updates to STDOUT\n"
        "  --degree_zero            Add the Degree Zero reduction\n"
        "  --degree_one             Add the Degree One reduction\n"
        "  --neighborhood           Add the Neighborhood Removal reduction\n"
        "  --triangle               Add the Triangle reduction\n"
        "  --v_shape                Add the V-Shape reduction\n"
        "  --domination             Add the Domination reduction\n"
        "  --twin                   Add the Twin reduction\n"
        "  --simplicial_vertex      Add the Simplicial Vertex reduction\n"
        "  --simplicial_transfer    Add the Simplicial Vertex with Weight Transfer reduction\n"
        "  --funnel                 Add the Weight Funnel reduction\n"
        "  --unconfined             Add the Unconfined reduction\n"
        "  --extended_domination    Add the Extended Domination reduction\n"
        "  --critical_set           Add the Critical Set reduction\n"
        "  --struction_sparse       Run Struction allowig only reductions that reduce V and E\n"
        "  --struction_dense        Run Struction allowig any reductions that reduce V\n"
        "  \n",
        prog);
}

int main(int argc, char **argv)
{
    struct sigaction sa = {.sa_handler = handle_stop};
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    char *graph_path = NULL;
    char *solution_path = NULL;

    int verbose = 0;

    int n_rules = 0, a_rules = 1 << 4;
    reduction *Rules = malloc(sizeof(reduction) * a_rules);

    int struction_sparse = 0;
    int struction_dense = 0;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"graph", required_argument, 0, 'g'},
        {"solution", no_argument, 0, 's'},
        {"verbose", no_argument, 0, 'v'},
        {"degree_zero", no_argument, 0, DEGREE_ZERO},
        {"degree_one", no_argument, 0, DEGREE_ONE},
        {"neighborhood", no_argument, 0, NEIGHBORHOOD_REMOVAL},
        {"triangle", no_argument, 0, TRIANGLE},
        {"v_shape", no_argument, 0, V_SHAPE},
        {"domination", no_argument, 0, DOMINATION},
        {"twin", no_argument, 0, TWIN},
        {"simplicial_vertex", no_argument, 0, SIMPLICIAL_VERTEX},
        {"simplicial_transfer", no_argument, 0, SIMPLICIAL_WEIGHT_TRANSFER},
        {"funnel", no_argument, 0, FUNNEL},
        {"unconfined", no_argument, 0, UNCONFINED},
        {"extended_domination", no_argument, 0, EXTENDED_DOMINATION},
        {"critical_set", no_argument, 0, CRITICAL_SET},
        {"struction_sparse", no_argument, 0, STRUCTION_SPARSE},
        {"struction_dense", no_argument, 0, STRUCTION_DENSE},
        {0, 0, 0, 0}};

    int opt;
    while ((opt = getopt_long(argc, argv, "hg:s:v", long_options, NULL)) != -1)
    {
        if (n_rules == a_rules)
        {
            a_rules *= 2;
            Rules = realloc(Rules, sizeof(reduction) * a_rules);
        }
        switch (opt)
        {
        case 'h':
            print_help(argv[0]);
            return 0;
        case 'g':
            graph_path = optarg;
            break;
        case 's':
            solution_path = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
        case DEGREE_ZERO:
            Rules[n_rules++] = degree_zero;
            break;
        case DEGREE_ONE:
            Rules[n_rules++] = degree_one;
            break;
        case NEIGHBORHOOD_REMOVAL:
            Rules[n_rules++] = neighborhood_removal;
            break;
        case TRIANGLE:
            Rules[n_rules++] = triangle;
            break;
        case V_SHAPE:
            Rules[n_rules++] = v_shape;
            break;
        case DOMINATION:
            Rules[n_rules++] = domination;
            break;
        case TWIN:
            Rules[n_rules++] = twin;
            break;
        case SIMPLICIAL_VERTEX:
            Rules[n_rules++] = simplicial_vertex;
            break;
        case SIMPLICIAL_WEIGHT_TRANSFER:
            Rules[n_rules++] = simplicial_vertex_with_weight_transfer;
            break;
        case FUNNEL:
            Rules[n_rules++] = weighted_funnel;
            break;
        case UNCONFINED:
            Rules[n_rules++] = unconfined;
            break;
        case EXTENDED_DOMINATION:
            Rules[n_rules++] = extended_domination;
            break;
        case CRITICAL_SET:
            Rules[n_rules++] = critical_set;
            break;
        case STRUCTION_SPARSE:
            struction_sparse = 1;
            break;
        case STRUCTION_DENSE:
            struction_dense = 1;
            break;

        default:
            return 1;
        }
    }

    if (graph_path == NULL)
    {
        printf("Missing graph path\n");
        return 0;
    }

    double t_io_0 = get_wtime();

    FILE *f = fopen(graph_path, "r");
    if (f == NULL)
    {
        printf("Failed to open %s\n", graph_path);
        return 1;
    }
    graph *g = graph_parse(f);
    graph_construction_sort_edges(g);
    fclose(f);

    double t_io_1 = get_wtime();

    int offset = 0, p = 0;
    while (graph_path[p] != '\0')
    {
        if (graph_path[p] == '/')
            offset = p + 1;
        p++;
    }

    long long n = g->n, m = g->m;

    if (verbose)
    {
        printf("Parsed %s with V=%lld E=%lld in %lfs\n", graph_path + offset, n, m / 2, t_io_1 - t_io_0);
    }

    reducer *r = reducer_init_list(g, n_rules, Rules);
    r->verbose = verbose;

    reduction_log *l = reducer_init_reduction_log(g);
    reducer_queue_all(r, g);

    double start = get_wtime();

    reducer_reduce_continue(r, g, l);

    long long struction_V = 0, struction_E = 0;

    if (keep_running && (struction_sparse || struction_dense))
    {
        struction_V = g->nr, struction_E = g->m;

        reducer *rs = reducer_init(g, 7, degree_zero, degree_one, neighborhood_removal, triangle, v_shape, twin, domination);

        if (keep_running && struction_sparse)
            struction_run(g, rs, l, 1, verbose);

        if (keep_running && struction_dense)
            struction_run(g, rs, l, 0, verbose);

        struction_V -= g->nr;
        struction_E -= g->m;

        reducer_free(rs);
    }

    double elapsed = get_wtime() - start;

    if (!verbose)
    {
        printf("\r%45s %10lld %10lld %10lld %10lld %10lld %8.4lf\n",
               graph_path + offset, n, m / 2, g->nr, g->m / 2, l->offset, elapsed);
    }
    else
    {
        printf("Elapsed time %lfs\n", elapsed);
        printf("Reduced size V=%lld E=%lld\n", g->nr, g->m / 2);

        if (g->nr == 0)
            printf("Found optimal solution with weight %lld\n", l->offset);
        else
            printf("Solution offset %lld\n", l->offset);

        for (int i = 0; i < 42; i++)
            printf("-");
        printf("\n");
        printf("%20s %10s %10s\n", "Reduction", "Reduced V", "Reduced E");
        for (int i = 0; i < n_rules; i++)
        {
            printf("%20s %10lld %10lld\n", Rules[i].name, r->Rule_impact_V[i], r->Rule_impact_E[i]);
        }
        if (struction_sparse || struction_dense)
            printf("%20s %10lld %10lld\n", struction.name, struction_V, struction_E);
    }

    if (solution_path != NULL && g->nr == 0)
    {
        int *I = malloc(sizeof(int) * g->n);
        for (node_id i = 0; i < g->n; i++)
            I[i] = 0;

        reducer_lift_solution(l, I);

        f = fopen(solution_path, "w");
        if (f == NULL)
        {
            printf("Failed to open %s\n", solution_path);
            return 1;
        }

        for (node_id u = 0; u < n; u++)
            if (I[u])
                fprintf(f, "%d\n", u + 1);

        fclose(f);

        free(I);
    }

    // reducer_restore_graph(g, l, 0);

    // assert(g->n == n && g->m == m);

    reducer_free_reduction_log(l);
    reducer_free(r);

    free(Rules);

    graph_free(g);

    return 0;
}