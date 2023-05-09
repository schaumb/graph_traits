//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_SEARCH_HPP
#define BXLX_GRAPH_SEARCH_HPP

#include "constants.hpp"

namespace bxlx::graph {

struct edge_types {
  enum type { tree, forward, reverse, cross, parallel };
  using tree_t = std::integral_constant<type, tree>;
  using forward_t = std::integral_constant<type, forward>;
  using reverse_t = std::integral_constant<type, reverse>;
  using cross_t = std::integral_constant<type, cross>;
  using parallel_t = std::integral_constant<type, parallel>;
};
using edge_type = edge_types::type;

/*
template<class G, class Traits, class ...Args>
struct edge_like_struct {
  optional<node_t<G, Traits>> parent;
  node_t<G, Traits> to;
  optional<reference_wrapper<const node_property_t<G, Traits>>> to_property;
  optional<edge_repr_t<G, Traits>> edge_repr;
  Args ... other_members;
};
 */


template<class Dist = size_t, class OutIt, class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr OutIt depth_first_search(G const& g, node_t<G, Traits> from,
                                   OutIt out, Dist max_dist = ~Dist()) {
  return out;
}

template<class Dist = size_t, class OutIt, class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr OutIt breadth_first_search(G const& g, node_t<G, Traits> from,
                                     OutIt out, Dist max_dist = ~Dist()) {
  return out;
}

template<class Weight = std::nullptr_t, class WeightRes = std::size_t,
          class OutIt, class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr OutIt shortest_paths(G const& g, node_t<G, Traits> from,
                               OutIt out, Weight = {},
                               WeightRes max_weight = ~WeightRes()) {
  return out;
}

}

#endif //BXLX_GRAPH_SEARCH_HPP
