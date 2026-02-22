# How to Add a New Reduction Rule

This document provides a tutorial on how to implement a new reduction rule and integrate it into the project.

A reduction rule is defined by a set of functions and a struct that holds pointers to these functions. The main logic is in the `reduce` function, which attempts to apply the reduction to the graph. If successful, it modifies the graph and stores any necessary information for later reconstructing the solution.

## 1. The `reduction` Struct

A reduction rule is represented by the `reduction` struct (defined in `include/reduction.h`):

```c
typedef struct
{
    func_reduce_graph reduce;
    func_reconstruct_solution reconstruct;
    func_clean clean;
    int global;
    const char *name;
} reduction;
```

*   `reduce`: A function pointer that attempts to apply the reduction at a given vertex.
*   `reconstruct`: A function pointer that lifts the solution from the reduced graph back to the original graph.
*   `clean`: A function pointer to free any memory allocated during the reduction.
*   `global`: A flag indicating if the reduction is a global one (applied once to the whole graph) or a local one (applied vertex by vertex).
*   `name`: The name of the reduction rule.

## 2. Implementing the Reduction Functions

You need to implement three functions for your reduction rule. Let's say your rule is named `my_rule`. You would implement:

*   `my_rule_reduce_graph`
*   `my_rule_reconstruct_solution`
*   `my_rule_clean`

### The `reduce` function

```c
int my_rule_reduce_graph(graph *g, node_id u, node_weight *offset, buffers *b, changed_list *c, reconstruction_data *d);
```

This is where the main logic of your reduction lies.

*   **Parameters**:
    *   `g`: The graph.
    *   `u`: The vertex to attempt the reduction on.
    *   `offset`: If the reduction is successful, this pointer should be updated with the change in the solution weight.
    *   `b`: A set of reusable buffers to avoid memory allocations.
    *   `c`: A list to store vertices whose neighborhoods have been modified (candidates for further reductions).
    *   `d`: A struct to store data for the reconstruction phase.
*   **Return value**: `1` if the reduction was successful, `0` otherwise.
*   **Responsibilities**:
    1.  Check if the reduction can be applied at vertex `u`.
    2.  If so, modify the graph using the functions from `include/graph.h` (e.g., `graph_remove_vertex`, `graph_remove_neighborhood`, `graph_add_edge`). Never modify the graph data directly, e.g., don't write `g->W[u] -= g->W[v]`, instead use `graph_change_vertex_weight`. This will ensure all changes can be correctly undone.
    3.  Update the `offset`.
    4.  Store any necessary data for the `reconstruct` function in the `reconstruction_data` struct `d`. If the provided fields are not enough, you can allocate memory and store the pointer in `d->data`.
    5.  Add all vertices whose neighborhoods have changed to the `changed_list` `c`. You can use the helper functions `reduction_data_queue_distance_one` and `reduction_data_queue_distance_two`.

### The `reconstruct` function

```c
void my_rule_reconstruct_solution(int *I, reconstruction_data *d);
```

This function is called during the solution lifting phase. It should use the data stored in `d` and `I` to determine the solution for the vertices that were removed/modified by this reduction.

*   `I`: An array representing the independent set of the reduced graph. Your function should update `I` for the vertices involved in your reduction.

### The `clean` function

```c
void my_rule_clean(reconstruction_data *d);
```

If you allocated any memory in your `reduce` function and stored it in `d->data`, you must free it here.

## 3. Create Header and Source Files

Create `include/reductions/my_rule.h` and `src/reductions/my_rule.c`.

**`include/reductions/my_rule.h`:**
This file should declare your three functions and define your `reduction` struct instance.

```c
#pragma once

#include "reduction.h"

int my_rule_reduce_graph(graph *g, node_id u, node_weight *offset, buffers *b, changed_list *c, reconstruction_data *d);
void my_rule_reconstruct_solution(int *I, reconstruction_data *d);
void my_rule_clean(reconstruction_data *d);

static const reduction my_rule = {
    .reduce = my_rule_reduce_graph,
    .reconstruct = my_rule_reconstruct_solution,
    .clean = my_rule_clean,
    .global = 0,
    .name = "my_rule"
};
```

**`src/reductions/my_rule.c`:**
This file will contain the implementation of your three functions.

```c
#include "my_rule.h"
#include <assert.h>

// ... your function implementations ...
```

## 4. Integrate the New Rule

To make your new rule available in the executable and library, you need to edit a few files:

1.  **`Makefile`**: Add `my_rule.o` to the `OBJ_SHARED` variable.
    ```makefile
    OBJ_SHARED = graph.o reducer.o reduction_data.o \
                 degree_zero.o degree_one.o ... \
                 my_rule.o
    ```

2.  **`include/mwis_reductions.h`**: Add `MY_RULE` to the `reduction_t` enum.
    ```c
    typedef enum {
        // ... existing rules
        MY_RULE,
        // ...
    } reduction_t;
    ```

3.  **`src/main.c`**:
    *   Include your new header: `#include "my_rule.h"`.
    *   Add a command-line option for your rule in `print_help()` and `long_options[]`.
    *   Add a case in the `switch` statement to add your rule to the list of active reductions.

4.  **`src/mwis_reductions.c`**:
    *   Include your new header: `#include "my_rule.h"`.
    *   Add a `case` for `MY_RULE` in the `switch` statement inside `mwis_reduction_reduce_graph()` to add your rule to the list of reductions when using the library.

## 5. Design Patterns and Best Practices

*   **Graph Modification**: Only modify the graph using the functions provided in `include/graph.h`. These functions ensure that changes are logged and can be reverted. Prefer `graph_remove_vertex` and `graph_remove_neighborhood` over manual edge deletions.
*   **Configuration**: The `include/defs.h` file contains macros for various initial allocation sizes and limits. If your reduction requires large buffers, you can add a new macro there.
*   **Reconstruction Data**: The `reconstruction_data` struct in `include/reduction_data.h` is designed to be lightweight. For simple rules, the existing fields `u, v, w, x, y, z` should be sufficient. For more complex rules, you can allocate your own struct, populate it, and assign its pointer to `d->data`. Remember to free this memory in your `.clean` function.
*   **State Changes**: When a reduction is successful, it's crucial to report which vertices have had their neighborhoods modified. This is done by adding them to the `changed_list` `c`. This ensures that subsequent reductions can be correctly applied to the neighbors. Use `reduction_data_queue_distance_one` and `reduction_data_queue_distance_two` for this.
*   **Signal Handling**: For particularly long-running reductions, consider checking the volatile `keep_running` flag declared in `defs.h`. When the program receives a SIGTERM signal, the reduction process should halt safely. If this happens, return 0 as if the reduction failed.
