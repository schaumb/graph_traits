//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_CONSTANTS_HPP
#define BXLX_GRAPH_CONSTANTS_HPP

#include "../recognize/graph_traits.hpp"

namespace bxlx::graph {
using traits::representation_t;
using traits::graph_traits;

template <class G, class Traits = graph_traits<G>, class = void>
constexpr bool is_graph_v = false;
template <class G, class Traits>
constexpr bool is_graph_v<G, Traits, std::void_t<decltype(Traits::representation)>> = true;

template <class G, class Traits = graph_traits<G>, bool V = is_graph_v<G, Traits>>
constexpr bool it_is_a_graph_v = V;
template <class G, class Traits>
constexpr bool it_is_a_graph_v<G, Traits, false> = [] () -> bool {
  return assert_types::why_not_graph<typename Traits::reason_t>{};
} ();

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr representation_t representation_v{};
template <class G, class Traits>
constexpr representation_t representation_v<G, Traits, true> = Traits::representation;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool has_graph_property_v = false;
template <class G, class Traits>
constexpr bool has_graph_property_v<G, Traits, true, std::void_t<typename Traits::graph_property::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool has_node_property_v = false;
template <class G, class Traits>
constexpr bool has_node_property_v<G, Traits, true, std::void_t<typename Traits::node_property::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool has_edge_property_v = false;
template <class G, class Traits>
constexpr bool has_edge_property_v<G, Traits, true, std::void_t<typename Traits::edge_property::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool is_user_defined_node_type_v = Traits::user_defined_node_t::value;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool is_user_defined_edge_type_v = Traits::user_defined_edge_t::value;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool has_node_container_v = false;
template <class G, class Traits>
constexpr bool has_node_container_v<G, Traits, true, std::void_t<typename Traits::node_container::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool has_adjacency_container_v = false;
template <class G, class Traits>
constexpr bool has_adjacency_container_v<G, Traits, true, std::void_t<typename Traits::adjacency_conatiner::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool has_edge_list_container_v = false;
template <class G, class Traits>
constexpr bool has_edge_list_container_v<G, Traits, true, std::void_t<typename Traits::edge_list_container::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool has_edge_container_v = false;
template <class G, class Traits>
constexpr bool has_edge_container_v<G, Traits, true, std::void_t<typename Traits::edge_container::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool has_in_adjacency_container_v = false;
template <class G, class Traits>
constexpr bool has_in_adjacency_container_v<G, Traits, true, std::void_t<typename Traits::in_adjacency_conatiner::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool has_in_edges_v = Traits::in_edges_t::value;

/*
template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool parallel_edges_v = ...;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool directed_edges_v = ...;
*/

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool compressed_edges_v = Traits::compressed_edges::type::value;


}

#endif //BXLX_GRAPH_CONSTANTS_HPP
