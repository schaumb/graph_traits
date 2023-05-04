//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_HPP
#define BXLX_GRAPH_TRAITS_HPP

#include "recognize.hpp"

namespace bxlx::graph::traits {
enum class representation_t {
  adjacency_list = 1,
  adjacency_matrix,
  edge_list,
};

template <class G, class V = decltype(state_machine::graph::template valid<G>())>
constexpr bool is_graph_v {V{}};

template <class G, class V = decltype(state_machine::graph::template valid<G>())>
constexpr bool it_is_a_graph_v = [] () -> bool {
  return assert_types::why_not_graph<V>::value;
} ();

template <class G, class V = decltype(state_machine::graph::template valid<G>()), bool = V{}>
struct graph_traits {
  using reason_t = V;
};

template <class G, class V>
struct graph_traits<G, V, true> {
  using properties_t = typename V::properties;

  constexpr static representation_t representation =
        properties_t{}.template has_property<state_machine::adj_list>()
              ? representation_t::adjacency_list :
        properties_t{}.template has_property<state_machine::adj_mat>()
              ? representation_t::adjacency_matrix :
        properties_t{}.template has_property<state_machine::edge_list>()
              ? representation_t::edge_list :
        representation_t{};

  static_assert(representation != representation_t{});
};


template <class G, bool = it_is_a_graph_v<G>>
[[maybe_unused]] constexpr representation_t representation_v{};
template <class G>
[[maybe_unused]] constexpr representation_t representation_v<G, true> = graph_traits<G>::representation;

} // namespace bxlx::graph::traits

#endif //BXLX_GRAPH_TRAITS_HPP
