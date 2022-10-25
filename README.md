# Graph traits
C++17 Graph traits and algorithms

---

## Quick overview

This `graph_traits` library recognizes 4 graph representation type automatically from c++ classes:
```cpp
enum class graph_representation {
    adjacency_list, // range of range node
    adjacency_set // range of set/bitset 
    adjacency_array, // pair of node in a list
    adjacency_matrix, // range of range bool
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
  - random access range output to copy/iterate the whole data efficiently (default)
  - input iterator range when not needed the whole data in memory, like for `take_while` or `drop_while` algorithms.
    - activated when you pass `bxlx::execution::lazy` execution argument.


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

---

```cpp
template<class Graph, class GraphTraits = /* traits */>
constexpr /* range<bfs_res> */ breadth_first_search(Graph&& graph, node_index_t start_index);

template<class ExPcy, class Graph, class GraphTraits = /* traits */>
/* range<bfs_res> */ breadth_first_search(ExPcy&& policy, Graph&& graph, node_index_t start_index);

template<class Graph, class GraphTraits = /* traits */, class OutputIterator>
constexpr OutputIterator breadth_first_search(Graph&& graph, node_index_t start_index, 
                                              OutputIterator out);

template<class ExPcy, class Graph, class GraphTraits = /* traits */, class OutputIterator>
OutputIterator breadth_first_search(ExPcy&& policy, Graph&& graph, 
                                    node_index_t start_index, OutputIterator out);
```

---

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


---

## More examples to graph mapping

### `adjacency_list`

- `random_access_range<sequence_range<integer>>`
- `random_access_range<sequence_range<pair<integer, edge_prop>>>`
- `random_access_range<pair<sequence_range<integer>, node_prop>>`
- `pair<random_access_range<sequence_range<integer>>, graph_prop>`
- node/edge/graph properties any combination

### `adjacency_set`
- `random_access_range<bitset_like>`
- `random_access_range<pair<bitset_like, node_prop>>`
- `pair<random_access_range<bitset_like>, graph_prop>`
- node/graph properties combination

*bitset_like: std::bitset<>, std::vector&lt;bool&gt;*

### `adjacency_array`

- `map<node_index, node_index>` // deprecated, probably it will be deleted
- `sequence_range<pair<node_index, node_index>>`
- `sequence_range<tuple<node_index, node_index, edge_prop>>`
- `pair<sequence_range<tuple<node_index, node_index>>, graph_prop>`
- edge/graph properties combination

### `adjacency_matrix`

- `random_access_range<random_access_range<bool>>`
- `random_access_range<random_access_range<optional<edge_prop>>>`
- `random_access_range<pair<random_access_range<bool>, node_prop>>`
- `pair<random_access_range<random_access_range<bool>>, graph_prop>`
- node/edge/graph properties any combination

---

## coming

### `adjacency_list`

- `map<node_index, sequence_range<node_index>`
- `map<node_index, sequence_range<pair<node_index, edge_prop>>`
- `map<node_index, pair<sequence_range<node_index>, node_prop>>`

### `adjacency_set`

- `random_access_range<set<integer>>` // current impl handles as adj_list.
- `random_access_range<map<integer, edge_prop>>` // current impl handles as adj_list.
- `map<node_index, set<node_index>>`
- `map<node_index, map<node_index, edge_prop>>`

### `adjacency_array`

- `pair<random_access_range<node_prop>, forward_range<pair<integer, integer>>>`
- `pair<random_access_range<node_prop>, forward_range<tuple<integer, integer, edge_prop>>>`


