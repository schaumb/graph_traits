
#include <bxlx/graph>

#include <iostream>
#include <string>


namespace common {
  template<class T>
  void run_example(T& t, std::ostream& o = std::cout) {
    using namespace bxlx::graph;

    switch (representation_v<T>) {
    case representation_t::adjacency_list:
      o << "Recognized adjacency list\n";
      break;
    case representation_t::adjacency_matrix:
      o << "Recognized adjacency matrix\n";
      break;
    case representation_t::edge_list:
      o << "Recognized edge list\n";
      break;
    }

    o << "\nTopological sort:\n";

    std::vector<node_t<T>> arr(node_count(t));
    topological_sort(t, arr.begin());

    for (auto& n : arr)
      o << n << ", ";
    o << "\n";
  }
}