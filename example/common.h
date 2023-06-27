
#include <bxlx/graph>

#include <iostream>
#include <string>
#include "magic_enum.hpp"


namespace common {
  template<class G>
  void run_example(G& g) {
    using namespace bxlx::graph;

    std::ostream& o = std::cout;
    o << "Recognized " << magic_enum::enum_name(representation_v<G>);

    o << "\n\nTopological sort:\n";
    try {
      std::vector<node_t<G>> arr(node_count(g));
      topological_sort(g, arr.begin());

      for (auto& n : arr) o << n << ", ";
    }
    catch (std::logic_error const& err) {
      o << "Failed: " << err.what();
    }

    o << "\n\nIs weakly connected:\t" << std::boolalpha << is_connected(g, std::false_type{})
      <<   "\nIs strongly connected:\t"                 << is_connected(g);

    std::vector<std::tuple<node_t<G>, node_t<G>, edge_type>> edges;
    node_t<G> start = *std::begin(node_indices(g));

    depth_first_search(g, start, std::back_inserter(edges));
    
    o << "\n\nEdge types at depth first search from start node: " << start << "\n";
    for (auto& [from, to, edge] : edges) {
      o << from << " -> " << to << ": " << magic_enum::enum_name(edge) << "\n";
    }
  }
}