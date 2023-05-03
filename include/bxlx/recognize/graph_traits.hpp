//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_HPP
#define BXLX_GRAPH_TRAITS_HPP

namespace bxlx::graph::traits {
enum class representation_t {
  adjacency_list = 1,
  adjacency_matrix,
  edge_list,
};

template <class G>
struct graph_traits;


template <class G>
[[maybe_unused]] constexpr representation_t representation_v = graph_traits<G>::representation;

template <typename G, typename Traits = graph_traits<G>>
constexpr bool is_graph_v = {};

template <typename G, typename Traits = graph_traits<G>>
constexpr bool it_is_a_graph_v = {};

} // namespace bxlx::graph::traits

#endif //BXLX_GRAPH_TRAITS_HPP
