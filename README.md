# Graph traits
C++17 Graph traits and algorithms

---

## Quick overview

This `graph_traits` library recognizes 3 graph representation type automatically from c++ classes:
```cpp
enum class graph_representation {
    adjacency_list, // range of range node
    adjacency_array, // pair of node in a list
    adjacency_matrix // range of range bool
};
```


Some example:
```cpp
static_assert(graph_traits<vector<vector<int>>>::representation == adjacency_list);

static_assert(graph_traits<bool[10][10]>::representation == adjacency_matrix);

static_assert(graph_traits<list<tuple<int, int, int>>>::representation == 
                    adjacency_array); // with bounded edge property

static_assert(graph_traits<
    tuple<vector<pair<list<optional<edge_prop>>, node_prop>>, graph_prop>
> == adjacency_matrix); // it has bounded edge, node and graph properties
```

---

The main concepts:
- all graph function allocate maximum once, at begin.
- if constexpr time known the nodes or edges (maximum) size, no heap allocation happens.
- overloaded functions with first argument `std::execution::*` is the parallel/vectorized algorithms.
- all function except parallel/vectorized overloads must be `constexpr`.
- multiple algorithm can be existing based on output iterator category.
  - random access range output to copy the whole data efficiently
  - input iterator range when not needed the whole data in memory, like filtered `take_while` or `drop_while` algorithms.


---


### Breadth first search

The result is always a range of this struct

```cpp
struct breadth_first_search_result {
    node_index_t parent;
    node_index_t index;
    size_t distance;
    edge_repr_type* edge;
    node_repr_type* node;
};
```

`*_repr_type` are depends on graph representation.

#### Example for `adjacency_array`

*current implementation is O(n\*e) where e is the edges count and n is the connected graph nodes count from start. This can be O((n + e)\*log(e)) later*

```cpp
#include <bxlx/graph_traits/algs/breadth_first_search.hpp>
#include <vector>
#include <string_view>
#include <utility>

void run_bfs() {
    std::vector<std::pair<std::string_view, std::string_view>> edge_list {
        {"x", "end"},
        {"a", "b"},
        {"b", "c"}, {"b", "e"}, {"b", "x"},
        {"e", "a"}, {"e", "c"}, {"e", "f"}
    }; 
    
    for (auto&& bfs_res : bxlx::graph::breadth_first_search(edge_list, "a")) {
        if (bfs_res.edge) // not the first node
            std::cout << bfs_res.parent << " -> ";
        std::cout << bfs_res.index << " (dist: " << bfs_res.distance << ")\n";
    }
    // possible output: 

    // a (dist: 0)
    // a -> b (dist: 1)
    // b -> x (dist: 2)
    // b -> c (dist: 2)
    // b -> e (dist: 2)
    // x -> end (dist: 3)
    // e -> f (dist: 3)
}
```

#### Example for `adjacency_list` with bounded node property

*O(n + e)*

```cpp
#include <bxlx/graph_traits/algs/breadth_first_search.hpp>
#include <array>
#include <initializer_list>
#include <string_view>

void run_bfs_2() {
    using namespace std;
    array<pair<initializer_list<int>, string_view>, 10> graph {{
        {{1, 2, 4, 9},  "node 0"},
        {{0, 5},        "node 1"},
        {{0, 1},        "node 2"},
        {{8, 9, 2},     "node 3"},
        {{7},           "node 4"},
        {{5, 5, 5},     "node 5"},
        {{0, 1, 2, 3},  "node 6"},
        {{3, 7, 9},     "node 7"},
        {{},            "node 8"},
        {{8},           "node 9"},
    }};
    for (auto&& bfs_res : bxlx::graph::breadth_first_search(graph, 2)) {
        if (bfs_res.edge) // not the first node
            cout << bfs_res.parent << " -> ";
        cout << "\"" << bfs_res.node->second << "\" "
                "(dist: " << bfs_res.distance << ")\n";
    }
    // output:
    
    // "node 2" (dist: 0)
    // 2 -> "node 0" (dist: 1)
    // 2 -> "node 1" (dist: 1)
    // 0 -> "node 4" (dist: 2)
    // 0 -> "node 9" (dist: 2)
    // 1 -> "node 5" (dist: 2)
    // 4 -> "node 7" (dist: 3)
    // 9 -> "node 8" (dist: 3)
    // 7 -> "node 3" (dist: 4)
}
```
