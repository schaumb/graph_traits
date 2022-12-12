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
>::representation == adjacency_matrix); // it has bounded edge, node and graph properties
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
using node_t = typename Traits::node_index_t;
template<class Traits>
using node_repr_t = typename Traits::node_repr_type;
template<class Traits>
using node_prop_t = typename Traits::node_property_type;


template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr bool has_node(const Graph&, const node_t<GraphTraits>&);

// for edge_list-s where no node property stored
template<class Graph, class GraphTraits = graph_traits<Graph>, 
         class Eq = std::equal_to<node_t<GraphTraits>>>
constexpr bool has_node(const Graph&, const node_t<GraphTraits>&, Eq&& = {});


template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
constexpr auto get_node(Graph [const|&|*] g, const node_t<GraphTraits>&)
    -> node_repr_t<GraphTraits> [const|&|*];


template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
constexpr auto get_node_property(Graph [const|&|*] g, const node_t<GraphTraits>&)
    -> node_prop_t<GraphTraits> [const|&|*];


// only for modifiable structures:

template<class Graph, class GraphTraits = graph_traits<Graph>      [, class ...Args           ]>
// parameters:                only if node index is user defined | only if it has node_property
constexpr auto add_node(Graph&[, const node_t<GraphTraits>&    ]   [, Args&& ...              ])
    -> std::pair<node_t<GraphTraits>, bool>; // index + was new


template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr bool remove_node(Graph&, const node_t<GraphTraits>&);
```

---

### Graph edge functions

`has_edge`, `get_edge`, `get_edge_property`, `add_edge`, `remove_edge` 

```cpp
// it has 2 separated namespace
namespace bxlx::graph::directed; // form->to
namespace bxlx::graph::undirected; // from - to (expects edge property sharing)

// whose has nested other namespace
namespace bxlx::graph::directed::multi;
namespace bxlx::graph::undirected::multi;


template<class Traits>
using edge_repr_t = typename Traits::edge_repr_type;
template<class Traits>
using edge_prop_t = typename Traits::edge_property_type;


template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr bool has_edge(const Graph&, const node_t<GraphTraits>&, const node_t<GraphTraits>&);

template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
constexpr auto get_edge(Graph [const|&|*] g, const node_t<GraphTraits>&, const node_t<GraphTraits>&)
    -> edge_repr_t<GraphTraits> [const|&|*];


template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
constexpr auto get_edge_property(Graph [const|&|*] g, const node_t<GraphTraits>&, const node_t<GraphTraits>&)
    -> edge_repr_t<GraphTraits> [const|&|*];


// only for modifiable structures:

template<class Graph, class GraphTraits = graph_traits<Graph>                          [, class ...Args           ]>
// parameters:                                                                         only if it has_edge_property
constexpr auto add_edge(Graph&, const node_t<GraphTraits>&, const node_t<GraphTraits>& [, Args&& ...              ])
    -> std::pair<edge_repr_t<GraphTraits>&, bool>; // edge_repr + was new

template<class Graph, class GraphTraits = graph_traits<Graph>>
constexpr bool remove_edge(Graph&, const node_t<GraphTraits>&, const node_t<GraphTraits>&);
```

---


### Breadth first search

---

#### Example for `edge_list`

```cpp
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
    
    for (auto&& [parent, index, distance] : bxlx::graph::shortest_paths(edge_list, "a")) {
        if (distance != 0) // not the first node
            std::cout << parent << " -> ";
        std::cout << index << " (dist: " << distance << ")\n";
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
    for (auto&& [parent, index, node, distance] : bxlx::graph::shortest_paths(graph, 2)) {
        if (distance != 0) // not the first node
            cout << parent << " -> ";
        cout << std::quoted(*node) <<
                " (dist: " << distance << ")\n";
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


- `node_indexed_range<map<integer, edge_prop>>`
- `node_indexed_range<pair<map<integer, edge_prop>, node_prop>>`
- `pair<node_indexed_range<map<integer, edge_prop>>, graph_prop>`
- node/graph properties any combination


These ranges cannot be any compile time fix range.
If range is a set like object, no different strategy (std::less<K> or std::equal_to<K>) is allowed for the keys

- `map<node_index, range<node_index>`
- `map<node_index, range<pair<node_index, edge_prop>>`
- `map<node_index, pair<range<node_index>, node_prop>>`
- `pair<map<node_index, range<node_index>, graph_prop>`
- node/edge/graph properties any combination


Map-s cannot be used by different strategy for keys (std::less<K> or std::equal_to<K>)

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
- `tuple<map<node_index, node_prop>, edge_range<pair<node_index, node_index>>>`
- `tuple<edge_range<tuple<node_index, node_index>>, graph_prop>`
- edge/node/graph properties combination


- `pair<node_indexed_range<node_prop>, edge_range<pair<integer, integer>>>`
- `pair<node_indexed_range<node_prop>, edge_range<tuple<integer, integer, edge_prop>>>`
- `tuple<node_indexed_range<node_prop>, edge_range<pair<integer, integer>>, graph_prop>`
- edge/graph properties any combination

---


## Algorithms, functions

### Non modifying algorithms

```cpp
struct edge_types {
  enum type { tree, forward, reverse, cross, parallel };
  using tree_t = std::integral_constant<type, tree>;
  using forward_t = std::integral_constant<type, forward>;
  using reverse_t = std::integral_constant<type, reverse>;
  using cross_t = std::integral_constant<type, cross>;
  using parallel_t = std::integral_constant<type, parallel>;
};
using edge_type = edge_types::type;

template<class ColoredEdgeOutIt, class Graph, class GraphTraits = ...>
constexpr ColoredEdgeOutIt depth_first_search(const Graph& g, node_t<Graph> from, ColoredEdgeOutIt out);
template<class ColoredEdgeOutIt, class Graph, class GraphTraits = ...>
constexpr ColoredEdgeOutIt breadth_first_search(const Graph& g, node_t<Graph> from, ColoredEdgeOutIt out);
// fills 'out' with a flexible class, which implicit convertible to:
// - std::tuple<node_t, node_t, const node_property_t*, TreeType, std::size_t>          - parent + to + node prop + TreeType + distance
// - std::tuple<node_t, node_t, const node_property_t*, TreeType>                       - parent + to + node prop + TreeType
// - std::tuple<node_t, node_t, TreeType, std::size_t>                                  - parent + to             + TreeType + distance
// - std::tuple<node_t, node_t, TreeType>                                               - parent + to             + TreeType
// - std::tuple<node_t, const node_property_t*, TreeType, std::size_t>                  -          to + node prop + TreeType + distance
// - std::tuple<node_t, const node_property_t*, TreeType>                               -          to + node prop + TreeType
// - std::tuple<node_t, TreeType, std::size_t>                                          -          to             + TreeType + distance
// - std::pair <node_t, TreeType>                                                       -          to             + TreeType

// where TreeType is one of the following type: 
// tree_t, forward_t, reverse_t, cross_t, parallel_t; whose implicit convertible to edge_type.


template<class Weight = EdgePropIdentityCmpOrSizeTOne, class OutIt, class Graph, class GraphTraits = ...>
constexpr OutIt shortest_paths(const Graph& g, node_t<Graph> from, OutIt out, Weight = {}, WeightRes max_weight = ~WeightRes());
// Weight is default if Graph has edge property, and the property has:
// - default constructor, (means zero weight/distance)
// - copy constructor
// - operator +(P, P)
// - operator <(P, P)
// - operator ~() on default constructed item.
// if edge property is not matching these properties, default SizeTOne is used as weight
//
// if Weight is user defined, any callable can pass which:
// - can be called with 3 argument: (node_t from, node_t to, const edge_property_t& prop)
// - can be called with 3 argument: (node_t from, node_t to, const edge_repr_t& prop)
// - can be called with 2 argument: (node_t from, node_t to)
// - can be called with 1 argument: (const edge_property_t& prop)
// - can be called with 1 argument: (const edge_repr_t& prop)
// - returns with a type whose match the listing above
// it must be sfinae friendly
//
// if Weight == SizeTOne, it uses bfs
// if Weight type is unsigned: T() < ~T(), or used bounded edge as weight, and all edge property is >= T() then dijkstra used
//    - signedness can be determined constexpr only if T [is trivial C++17] [has constexpr constructor/destructor and operator~ >=C++20]
// else use bellman_ford algorithm
// 
// fills 'out' with a flexible class, which implicit convertible to: (*)
// - std::tuple<node_t, node_t, const node_property_t*, const edge_property_t*, weight> - parent + to + n_property + e_property + weight
// - std::tuple<node_t, node_t, const node_property_t*, const edge_property_t*>         - parent + to + n_property + e_property
// - std::tuple<node_t, node_t, const edge_property_t*, weight>                         - parent + to              + e_property + weight
// - std::tuple<node_t, node_t, const edge_property_t*>                                 - parent + to              + e_property
// - std::tuple<node_t, node_t, const node_property_t*, weight>                         - parent + to + n_property              + weight
// - std::tuple<node_t, node_t, const node_property_t*>                                 - parent + to + n_property
// - std::tuple<node_t, node_t, weight>                                                 - parent + to                           + weight
// - std::tuple<node_t, const node_property_t*, weight>                                 -          to + n_property              + weight
// - std::pair <node_t, weight>                                                         -          to                           + weight
// if edge_property is not exists, those overloads works with edge_repr_t

template<class Weight = EdgePropIdentityCmpOrSizeTOne, class NodeInputIt, class OutIt, class Graph, class GraphTraits = ...>
constexpr OutIt shortest_paths(const Graph& g, NodeInputIt first, NodeInputIt last, OutIt out, Weight = {}, WeightRes max_weight = ~WeightRes());
// same as previous, but with multiple start node


template<class Graph, class GraphTraits = ...>
constexpr bool is_directed_acyclic(const Graph& g);
// A graph is directed acyclic if no cycle found in it


template<class Graph, class GraphTraits = ...>
constexpr bool is_connected(const Graph& g, bool is_directed = /*TODO*/);
template<class Graph, class GraphTraits = ...>
constexpr bool is_forest(const Graph& g, bool is_directed = /*TODO*/);

template<class Graph, class GraphTraits = ...>
constexpr bool is_tree(const Graph& g, bool is_directed = /*TODO*/);
// == is_forest && is_connected



template<class NodeOutIt, class Graph, class GraphTraits = ...>
constexpr NodeOutIt component(const Graph& g, node_t<Graph> node, NodeOutIt out);
// fills 'out' with node indices, whose connected to 'node' 

template<class NodeOutIt, class Graph, class GraphTraits = ...>
constexpr NodeOutIt topological_sort(const Graph& g, NodeOutIt out);


template<class Weight = EdgePropIdentityCmpOrSizeTOne, [class Heuristic,]
         class OutIt, class Graph, class GraphTraits = ...>
constexpr OutIt shortest_path(const Graph& g, node_t<Graph> from, node_t<Graph> to, 
                              OutIt out, Weight&& = {}[, Heuristic&& = {}]);
// Weight argument is same as in shortest_paths

// Heuristic:
// - can be called with (node_t from, node_t end)
// - has copy constructor
// - return value (r) has operator+(weight, r) -> weight


template<class Weight = EdgePropIdentityCmpOrSizeTOne, [class Heuristic,]
         class NodeInputIt, class OutIt, class Graph, class GraphTraits = ...>
constexpr OutIt shortest_path(const Graph& g, NodeInputIt first_from, NodeInputIt last_from, node_t<Graph> to, 
                              OutIt out, Weight&& = {} [, Heuristic&& = {}]);
// same as previous version, except this accept multiple from indices


template<class PairOutIt, class Color = std::size_t, class Graph, class GraphTraits = ...>
constexpr PairOutIt coloring(const Graph& g, PairOutIt out, Color max_color = ~Color());
// this algorithm fills 'out' with a pair of node index, and [Color(), ..., max_color) assigned color
// std::pair<node_t, Color>
// Color must be default constructible, copy constructible and has operator++ exists

//                                                                          adj_list    | adj_mat   | edge_list
// if max_color is INF (default, means no maximum), greedy algorithm used   O(n+e)      | ?         | ?
// if max_color == 2, bipartite algorithm used                              O(n+e)      | ?         | ?
// else [1] algorithm used                                                  O(c * n^2)  | ?         | ?

// throws invalid_argument if max(out_edge_count(node)..., in_edge_count(node)...) >= max_color 


template<class PairOutIt, class ComponentType = std::size_t, class Graph, class GraphTraits = ...>
constexpr PairOutIt components(const Graph& g, PairOutIt out, ComponentType start = ComponentType());
// fills 'out' with a pair of node index, and [start, ...) assigned component index
// std::pair<node_t, ComponentType>
// ComponentType must be copy constructible and has operator++ exists


template<class Weight = EdgePropIdentityCmpOrSizeTOne, class OutIt, class Graph, class GraphTraits = ...>
constexpr OutIt matching_edges(const Graph& g, OutIt out, Weight&& = {});
// fills 'out' with edges [minimum weight if set] whose match maximum possible node in the graph

template<class Weight = EdgePropIdentityCmpOrNone, class OutIt, class Graph, class GraphTraits = ...>
constexpr OutIt cover_edges(const Graph& g, OutIt out, Weight&& = {});
// fills 'out' with edges [minimum weight if set] whose cover all node in the graph
// it uses matching() and complements it

template<class Weight = EdgePropIdentityCmpOrNone, class OutIt, class Graph, class GraphTraits = ...>
constexpr OutIt spanning_edges(const Graph& g, OutIt out, Weight&& = {});
// fills 'out' with edges [minimum weight if set] whose used in graph it keeps the components connected 
// without any undirected circle (on a connected graph, it makes a tree)

template<class OutIt, class Graph, class GraphTraits = ...>
constexpr OutIt eulerian_path(const Graph& g[, node_t<Graph> start], OutIt out, bool closed = false);
// contains all edge from graph


namespace isomorph {
struct hash {
    using is_transparent = ...;
    template<class Graph>
    constexpr std::size_t operator()(const Graph&) noexcept(...) const;
    // uses [2]
};

struct equal_to {
    using is_transparent = ...;
    template<class G1, class G2>
    constexpr bool operator()(const G1&, const G2&) noexcept(...) const;
    // first uses node degree sorting comparator, then
    // uses [3]
};

}

// Currently not needed

template<class Graph, class GraphTraits = ...>
constexpr bool is_planar(const Graph&);
// A graph is planar iff it can be drawn in a plane without any edge intersections.

template<class Graph, class GraphTraits = ...>
constexpr bool is_regular(const Graph&[, std::size_t k = INF]);
// if k == INF then any regularity accepted, if not, only k regularity
// graph is k regular iff all node degree is exactly k


// [1] Kierstead, H. A., Kostochka, A. V., Mydlarz, M., & Szemerédi, E. (2010). A fast algorithm for equitable coloring. Combinatorica, 30(2), 217-224.
// [2] Shervashidze, Nino, Pascal Schweitzer, Erik Jan Van Leeuwen, Kurt Mehlhorn, and Karsten M. Borgwardt. Weisfeiler Lehman Graph Kernels. Journal of Machine Learning Research. 2011. http://www.jmlr.org/papers/volume12/shervashidze11a/shervashidze11a.pdf
// [3] L. P. Cordella, P. Foggia, C. Sansone, M. Vento, "An Improved Algorithm for Matching Large Graphs", 3rd IAPR-TC15 Workshop  on Graph-based Representations in Pattern Recognition, Cuen, pp. 149-159, 2001.
```

#### Ranges

```cpp
namespace ranges {
// + BFS, DFS


template<class OutType = void, class Weight = EdgePropIdentityCmpOrSizeTOne, class Graph,
         class StorageVector = Storage<OutType, Graph>,
         class GraphTraits = ...>
constexpr StorageVector shortest_paths(const Graph& g, node_t<Graph> from, Weight = {}, 
                                       WeightRes max_weight = ~WeightRes(),
                                       StorageVector&& = {});
// returns std::vector<std::tuple<node_t, node_t, weight>> if OutType == void and no bounded property
// returns std::vector<std::tuple<node_t, node_t, const node_property_t*, weight>> if OutType == void, and it has bounded node property 
// returns std::vector<OutType> if OutType is constructible from any acceptable edge output, see (*)
// returns OutType if it is a range of acceptable edge output
// if Graph node size is constexpr, then returns default std::array<T, NODES> replacement of std::vector
        // the unfilled array elements will contain GraphTraits::invalid TO nodes 

        
template<class OutType = void, class Weight = EdgePropIdentityCmpOrSizeTOne, class InputNodes, 
         class Graph, class StorageVector = Storage<OutType, Graph>, 
         class GraphTraits = ...>
constexpr StorageVector shortest_paths(const Graph& g, InputNodes&& from, Weight = {}, 
                                       WeightRes max_weight = ~WeightRes(),
                                       StorageVector&& = {});


template<class NodeVector = void, class Graph, 
         class StorageVector = Storage<NodeVector, Graph, node_t<Graph>>,
         class GraphTraits = ...>
constexpr StorageVector component(const Graph& g, node_t<Graph> node, StorageVector&& = {});


template<class NodeVector = void, class Graph, 
         class StorageVector = Storage<NodeVector, Graph, node_t<Graph>>, 
         class GraphTraits = ...>
constexpr StorageVector topological_sort(const Graph& g, StorageVector&& = {});



template<class OutType = void, class Weight = EdgePropIdentityCmpOrSizeTOne, [class Heuristic,]
         class Graph, class StorageVector = Storage<OutType, Graph>, class GraphTraits = ...>
constexpr StorageVector shortest_path(const Graph& g, node_t<Graph> from, node_t<Graph> to, 
                                      StorageVector&& = {}, Weight&& = {}[, Heuristic&& = {}]);


template<class OutType = void, class Weight = EdgePropIdentityCmpOrSizeTOne, [class Heuristic,]
         class InputNodes, class Graph, class StorageVector = Storage<OutType, Graph>, 
         class GraphTraits = ...>
constexpr StorageVector shortest_path(const Graph& g, InputNodes&& from, node_t<Graph> to, 
                                      StorageVector&& = {}, Weight&& = {}[, Heuristic&& = {}]);


template<class PairVector = void, class Color = std::size_t, class Graph, 
         class StorageVector = Storage<PairVector, Graph, std::pair<node_t<Graph>, Color>>,
         class GraphTraits = ...>
constexpr StorageVector coloring(const Graph& g, Color max_color = ~Color(), StorageVector&& = {});


template<class PairVector = void, class ComponentType = std::size_t, class Graph, 
         class StorageVector = Storage<PairVector, Graph, std::pair<node_t<Graph>, ComponentType>>
         class GraphTraits = ...>
constexpr StorageVector components(const Graph& g, ComponentType start = ComponentType(), StorageVector&& = {});



template<class OutType = void, class Weight = EdgePropIdentityCmpOrSizeTOne, class Graph, 
         class StorageVector = Storage<OutType, Graph>,
         class GraphTraits = ...>
constexpr StorageVector X(const Graph& g, Weight&& = {}, StorageVector&& = {});
// where X can be: matching_edges, cover_edges, spanning_edges


template<class OutType = void, class Graph, 
         class StorageVector = Storage<OutType, Graph>,
         class GraphTraits = ...>
constexpr StorageVector eulerian_path(const Graph& g[, node_t<Graph> start], bool closed = false,
                                      StorageVector&& = {});
}
```