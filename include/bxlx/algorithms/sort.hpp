//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_SORT_HPP
#define BXLX_GRAPH_SORT_HPP
#include "bxlx/algorithms/detail/getters.hpp"

#include <sstream>

namespace bxlx::graph {

template<class OutIt, class G, class Traits = graph_traits<G>, class NodeSet =
  detail::node_set_t<G, Traits, std::integral_constant<std::size_t, 3>>>
constexpr OutIt topological_sort(
      G const& g, OutIt out, NodeSet&& nodes = {}) {
  auto it = std::make_reverse_iterator(std::next(out, node_count(g)));
  using ItT = decltype(it);

  struct out_it_t {
    ItT& it;

    constexpr out_it_t& operator++() {
      return *this;
    }

    constexpr out_it_t& operator++(int) {
      return *this;
    }

    constexpr out_it_t& operator*() {
      return *this;
    }

    [[noreturn]]
    const out_it_t& operator=(std::tuple<node_t<G, Traits>, node_t<G, Traits>, edge_types::reverse_t> const& v) {
      std::stringstream ss;

      ss << "Not a DAG, found circle: " << std::get<0>(v) << " -> " << std::get<1>(v) << " -~> " << std::get<0>(v);

      bxlx::graph::detail::throw_or_terminate<std::logic_error>(ss.str());
    }

    constexpr const out_it_t& operator=(std::tuple<node_t<G, Traits>, node_types::post_visit_t, size_t> const& v) {
      *it++ = std::get<0>(v);
      return *this;
    }
  } out_it{it};

  for (node_t<G, Traits> node : node_indices(g)) {
    const auto current_state = [] (NodeSet& nodes, node_t<G, Traits> const& to_node) {
      if constexpr (type_traits::range_type_v<NodeSet> == type_traits::range_type_t::set_like) {
        return nodes.count(to_node);
      } else {
        return nodes[to_node];
      }
    } (nodes, node);
    if (current_state == detail::white) {
      depth_first_search(g, node, out_it, nodes);
    }
  }
  return out;
}
}

#endif //BXLX_GRAPH_SORT_HPP
