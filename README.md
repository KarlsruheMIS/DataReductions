# Data Reductions for the Maximum Weight Independent Set problem

Part of the [KarlsruheMIS](https://github.com/KarlsruheMIS) organization.

This project provides a C library with a collection of reduction rules for the Maximum Weight Independent Set (MWIS) problem. It also includes a command-line executable to apply these reductions to a given graph.

This code is intended as a reference implementation for the reduction rules described in our survey paper on arXiv, [Accelerating Reductions Using Graph Neural Networks and a New Concurrent Local Search for the Maximum Weight Independent Set Problem](https://doi.org/10.48550/arXiv.2412.14198) by Ernestine Großmann, Kenneth Langedal, and Christian Schulz.

For information on how to contribute, see the [REDUCTIONS.md](REDUCTIONS.md) file. Note that this is a work in progress. If you encounter any problems with the code, please let us know by opening a new issue or sending us an email.

## How to Build

To build the project, simply run `make`:

```bash
make
```

This will create two main artifacts:

1.  `mwis_reduce`: An executable to apply the reduction rules.
2.  `libmwis_reductions.a`: A static library that can be linked against in other projects.

## How to Use the Executable

The `mwis_reduce` executable reads a graph in the METIS format, applies a set of specified reduction rules, and prints statistics about the reduction process.

### Usage

```bash
./mwis_reduce -g <graph_file> [options]
```

### Options

*   `-h, --help`: Show the help message.
*   `-g, --graph <path>`: Path to the input graph in METIS format.
*   `-s, --solution <path>`: Path to store the solution if the graph is fully reduced (empty kernel).
*   `-v, --verbose`: Enable verbose output.

### Reduction Rule Options

You can specify which reduction rules to apply using the following flags:

*   `--degree_zero`
*   `--degree_one`
*   `--neighborhood`
*   `--triangle`
*   `--v_shape`
*   `--domination`
*   `--twin`
*   `--simplicial_vertex`
*   `--simplicial_transfer`
*   `--funnel`
*   `--unconfined`
*   `--extended_domination`
*   `--critical_set`
*   `--struction_sparse`: Run struction allowing only reductions that reduce |V| and |E|.
*   `--struction_dense`: Run struction allowing any reductions that reduce |V|.

### Example

To run a few reduction rules on a graph:
```bash
./mwis_reduce -g my_graph.graph --degree_one --domination --twin
```

Without the `--verbose` option, the printout will be a single line on the following format:

```bash
[graph name] [vertices] [edges] [kernel vertices] [kernel edges] [solution offset] [elapsed time (s)]
```

The executable does not provide a `timeout` option; however, it responds to SIGINT/SIGTERM signals and terminates safely when received. For timed experiments, run the executable under `timeout` to send SIGTERM after a given amount of time, for instance:

```bash
timeout 60s ./mwis_reduce -g my_graph.graph --degree_one --domination --twin
```

## How to Use the Library

The project can be used as a C/C++ library to integrate the reduction rules into your own projects.

### Linking

You need to include the main header file `mwis_reductions.h` and link against the static library `libmwis_reductions.a`.

When compiling your project, you'll need to add the `include` directory to your include paths and link with the library. For example, with `g++`:

```bash
g++ your_code.cpp -I /path/to/mwis_reductions.h -L /path/to/libmwis_reductions.a -lmwis_reductions
```

### C++ Example Usage

The following example shows how to use the library to load a graph, apply some reductions, and reconstruct the solution. See `examples/example.cpp` for a complete working example.

```cpp
#include <iostream>
#include <vector>
#include "mwis_reductions.h"

int main() {
    // Read a graph from a file
    mwis_reduction_graph *g = mwis_reduction_graph_read_from_file("path/to/your/graph.gr");

    // Define the reduction rules to apply
    std::vector<reduction_t> rules = {
        DEGREE_ZERO,
        DEGREE_ONE,
        DOMINATION
    };

    // Apply the reduction rules
    mwis_reduction_reduce_graph(g, rules.size(), rules.data());

    std.cout << "Graph reduced to V=" << g->nr << " and E=" << g->m << std::endl;
    std::cout << "Solution offset so far: " << g->offset << std::endl;

    // ... you can now run your own solver on the reduced graph ...
    // For this example, we assume an empty solution on the reduced graph.

    std::vector<int> solution(g->n, 0);

    // Lift the solution back to the original graph
    mwis_reduction_lift_solution(g, solution.data());

    // Restore the graph to its original state
    mwis_reduction_restore_graph(g);

    // The 'solution' vector now holds the independent set for the original graph
    // ...

    // Free the graph
    mwis_reduction_graph_free(g);

    return 0;
}
```

### How to Contribute

We aim to keep this repository up to date as new reduction rules are discovered. As such we provide a short tutorial on how to contribute new reductions [here](REDUCTIONS.md)