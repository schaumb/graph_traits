
#include <bxlx/graph>

#include <iostream>
#include <string>
#include "magic_enum.hpp"


namespace common {
  template<class T>
  void run_example(T& t, std::ostream& o = std::cout) {
    using namespace bxlx::graph;

    o << "Recognized " << magic_enum::enum_name(representation_v<T>);

    o << "\n\nTopological sort:\n";

    try {
      std::vector<node_t<T>> arr(node_count(t));
      topological_sort(t, arr.begin());

      for (auto& n : arr) o << n << ", ";
    }
    catch (std::logic_error const& err) {
      o << "Failed: " << err.what();
    }

    o << "\n\nIs weakly connected: " << std::boolalpha << is_connected(t, std::false_type{}) << "\n";
    o << "Is strongly connected: " << std::boolalpha << is_connected(t);

    std::vector<std::tuple<node_t<T>, node_t<T>, edge_type>> edges;

    node_t<T> start = *std::begin(node_indices(t));
    depth_first_search(t, start, std::back_inserter(edges), std::multiset<node_t<T>>{});
    
    o << "\n\nEdge types at depth first search from start node: " << start << "\n";
    for (auto& [from, to, edge] : edges) {
      o << from << " -> " << to << ": " << magic_enum::enum_name(edge) << "\n";
    }
  }
}