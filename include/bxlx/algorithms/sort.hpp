//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_SORT_HPP
#define BXLX_GRAPH_SORT_HPP
#include "getters.hpp"

namespace bxlx::graph {

template<class OutIt, class G, class Traits = graph_traits<G>, class NodeSet = detail::node_set_t<G, Traits>>
constexpr OutIt topological_sort(G const& g, OutIt out, NodeSet&& nodes = {}) {
  auto it = std::make_reverse_iterator(std::next(out, node_count(g)));
  using ItT = decltype(it);

  struct out_it_t {
    ItT& it;
    NodeSet& nodes;

    constexpr const out_it_t& operator++() const{
      ++it;
      return *this;
    }
    constexpr const out_it_t& operator*() const{
      return *this;
    }

    constexpr const out_it_t& operator=(std::pair<node_t<G, Traits>, edge_types::tree_t> const& v) const{
      *it++ = std::get<0>(v);
      return *this;
    }
  } out_it{it, nodes};
  for (node_t<G, Traits> node : node_indices(g)) {
    const auto current_state = [] (NodeSet& nodes, node_t<G, Traits> const& to_node) {
      if constexpr (type_traits::range_type_v<NodeSet> == type_traits::range_type_t::set_like) {
        return nodes.count(to_node);
      } else {
        return nodes[to_node];
      }
    } (nodes, node);
    if (current_state == detail::white) {
      depth_first_search(g, node, out_it);
    }
  }
  return out;
}
}

#endif //BXLX_GRAPH_SORT_HPP
