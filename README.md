# Graph traits
C++17 Graph traits and algorithms

---

## Quick overview

This `graph_traits` library recognizes 3 graph representation type automatically from c++ classes:
```cpp
enum class graph_representation {
    adjacency_list, // range of range node
    adjacency_matrix, // range of range bool
    edge_list, // pair of node in a list
};
```


Some example:
```cpp
static_assert(graph_traits<vector<vector<int>>>::representation == adjacency_list);

static_assert(graph_traits<bool[10][10]>::representation == adjacency_matrix);

static_assert(graph_traits<list<tuple<int, int, int>>>::representation == 
                    edge_list); // with bounded edge property

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

### Graph node functions

`has_node`, `get_node`, `get_node_property`, `add_node`, `remove_node`

```cpp
template<class Traits>
using node_t = typename GraphTraits::node_index_t;
template<class Traits>
using node_repr_t = typename GraphTraits::node_repr_type;
template<class Traits>
using node_prop_t = typename GraphTraits::node_property_type;


template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr bool has_node(const Graph&, const node_t<GraphTraits>&)

// for edge_list-s where no node property stored
template<class Graph, class GraphTraits = graph_traits<Graph>, 
         class Eq = std::equal_to<node_t<GraphTraits>>>
constexpr bool has_node(const Graph&, const node_t<GraphTraits>&, Eq&& = {})


template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
constexpr auto get_node(Graph [const|&|*] g, const node_t<GraphTraits>&)
    -> node_repr_t<GraphTraits> [const|&|*];


template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
constexpr auto get_node_property(Graph [const|&|*] g, const node_t<GraphTraits>&)
    -> node_prop_t<GraphTraits> [const|&|*];


// only for modifiable structures:

template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>> [, class ...Args]>
// parameters:                only if node index is user defined | only if it has node_property
constexpr auto add_node(Graph&[, const node_t<GraphTraits>&]       [, Args&& ...]              )
    -> std::pair<node_t<GraphTraits>, bool> // index + was new


template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
constexpr bool remove_node(Graph&, const node_t<GraphTraits>&);
```

---

### Graph edge functions

`has_edge`, `get_edge`, `get_edge_property`, `add_edge`, `remove_edge` 

```cpp
// it has 3 separated namespace
namespace bxlx::graph::directed; // form->to
namespace bxlx::graph::bidirectional; // from<->to (different edge properties)
namespace bxlx::graph::undirected; // from - to (expects edge property sharing)

template<class Traits>
using edge_repr_t = typename GraphTraits::edge_repr_type;
template<class Traits>
using edge_prop_t = typename GraphTraits::edge_property_type;


template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr bool has_edge(const Graph&, const node_t<GraphTraits>&, const node_t<GraphTraits>&);
    // bidirectional checks both direction

template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr auto get_edge(Graph [const|&|*] g, const node_t<GraphTraits>&, const node_t<GraphTraits>&)
    // directed/undirected
    -> edge_repr_t<GraphTraits> [const|&|*];
    // bidirectional
    -> std::pair<edge_repr_t<GraphTraits> [const|&|*], 
                 edge_repr_t<GraphTraits> [const|&|*]>;


template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr auto get_edge_property(Graph [const|&|*] g, const node_t<GraphTraits>&, const node_t<GraphTraits>&)
    // directed/undirected
    -> edge_repr_t<GraphTraits> [const|&|*];
    // bidirectional
    -> std::pair<edge_prop_t<GraphTraits> [const|&|*], 
                 edge_prop_t<GraphTraits> [const|&|*]>;


// only for modifiable structures:

// directed/undirected
template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>> [, class ...Args]>
// parameters:                                                                         only if it has_edge_property
constexpr auto add_edge(Graph&, const node_t<GraphTraits>&, const node_t<GraphTraits>& [, Args&& ...]              )
    -> std::pair<edge_repr_t<GraphTraits>&, bool> // edge_repr + was new

// bidirectional
template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>, class Tup1, class Tup2>
// parameters:                                                                         only if it has_edge_property
constexpr auto add_edge(Graph&, const node_t<GraphTraits>&, const node_t<GraphTraits>& [, Tup1&&, Tup2&&]          )
    -> std::pair<std::pair<edge_repr_t<GraphTraits>&,
                           edge_repr_t<GraphTraits>&>, bool> // edge_reprs + was new


template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr bool remove_edge(Graph&, const node_t<GraphTraits>&, const node_t<GraphTraits>&);
```

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

#### Example for `edge_list`

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

in the schema, the node_index/index must be the same

### `adjacency_list`

- `node_indexed_range<range<integer>>`
- `node_indexed_range<range<pair<integer, edge_prop>>>`
- `node_indexed_range<pair<range<integer>, node_prop>>`
- `pair<node_indexed_range<range<integer>>, graph_prop>`
- node/edge/graph properties any combination


- `map<node_index, range<node_index>`
- `map<node_index, range<pair<node_index, edge_prop>>`
- `map<node_index, pair<range<node_index>, node_prop>>`
- `pair<map<node_index, range<node_index>, graph_prop>`
- node/edge/graph properties any combination


- `map<node_index, map<node_index, edge_prop>>`
- `map<node_index, pair<map<node_index, edge_prop>, node_prop>>`
- `pair<map<node_index, map<node_index, edge_prop>>, graph_prop>`
- node/graph properties any combination

### `adjacency_matrix`

*bitset_like: std::bitset<>, std::vector&lt;bool&gt;*

- `node_indexed_range<bitset_like>`
- `node_indexed_range<pair<bitset_like, node_prop>>`
- `pair<node_indexed_range<bitset_like>, graph_prop>`
- node/graph properties combination


- `node_indexed_range<node_indexed_range<bool>>`
- `node_indexed_range<node_indexed_range<optional<edge_prop>>>`
- `node_indexed_range<pair<node_indexed_range<bool>, node_prop>>`
- `pair<node_indexed_range<node_indexed_range<bool>>, graph_prop>`
- node/edge/graph properties any combination

### `edge_list`

- `edge_range<pair<node_index, node_index>>`
- `edge_range<tuple<node_index, node_index, edge_prop>>`
- `pair<edge_range<tuple<node_index, node_index>>, graph_prop>`
- edge/graph properties combination


- `pair<node_indexed_range<node_prop>, edge_range<pair<integer, integer>>>`
- `pair<node_indexed_range<node_prop>, edge_range<tuple<integer, integer, edge_prop>>>`
- `tuple<node_indexed_range<node_prop>, edge_range<pair<integer, integer>>, graph_prop>`
- edge/graph properties any combination


- `pair<map<node_index, node_prop>, edge_range<pair<node_index, node_index>>>`
- `pair<map<node_index, node_prop>, edge_range<tuple<node_index, node_index, edge_prop>>>`
- `tuple<map<node_index, node_prop>, edge_range<pair<node_index, node_index>>, graph_prop>`
- edge/graph properties any combination

---

