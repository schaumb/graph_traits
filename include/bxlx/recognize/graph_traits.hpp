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

template <class G, class V = decltype(state_machine::graph::template valid<G>()), bool = V{}>
struct graph_traits {
  using reason_t = V;
};

template <class G, class V>
struct graph_traits<G, V, true> {
  using properties_t = typename V::properties;

  constexpr static representation_t representation =
        properties_t::template has_property_v<state_machine::adj_list>
              ? representation_t::adjacency_list :
        properties_t::template has_property_v<state_machine::adj_mat>
              ? representation_t::adjacency_matrix :
        properties_t::template has_property_v<state_machine::edge_list>
              ? representation_t::edge_list :
        representation_t{};

  static_assert(representation != representation_t{});

  constexpr static bool has_graph_property = properties_t::template has_property_v<state_machine::graph_prop_t>.value;
  constexpr static bool has_edge_property = properties_t::template has_property_v<state_machine::edge_prop_t>.value;
  constexpr static bool has_node_property = properties_t::template has_property_v<state_machine::node_prop_t>.value;

  constexpr static bool user_defined_node_type = !properties_t::template is_valid_v<state_machine::user_node_t, std::false_type>.value;

  constexpr static bool has_edge_type = !properties_t::template is_valid_v<state_machine::user_edge_t, state_machine::na_t>.value;

  constexpr static bool has_node_container = properties_t::template has_property_v<state_machine::node_container<next_type::Dummy>>.value;
  constexpr static bool has_edge_container = properties_t::template has_property_v<state_machine::edge_container<next_type::Dummy>>.value;

  constexpr static bool has_adjacency_container = properties_t::template has_property_v<state_machine::adj_list_cont<next_type::Dummy>>.value ||
                                                  properties_t::template has_property_v<state_machine::adj_mat_cont<next_type::Dummy>>.value ||
                                                  !properties_t::template is_valid_v<state_machine::compressed_t, std::false_type>.value;

  constexpr static bool has_edge_list_container = representation == representation_t::edge_list;

  constexpr static bool has_in_adjacency_container = properties_t::template has_property_v<state_machine::indexed_type<state_machine::adj_list_cont<next_type::Dummy>, 1>>.value ||
                                                     properties_t::template has_property_v<state_machine::indexed_type<state_machine::adj_mat_cont<next_type::Dummy>, 1>>.value;

  constexpr static bool has_in_edges = !properties_t::template is_valid_v<state_machine::in_edges_t, std::false_type>;

  constexpr static bool compressed_edges = !properties_t::template is_valid_v<state_machine::compressed_t, std::false_type>;
};

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
[[maybe_unused]] constexpr representation_t representation_v{};
template <class G, class Traits>
[[maybe_unused]] constexpr representation_t representation_v<G, Traits, true> = Traits::representation;

/*
#define BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(member) Traits::member


template <class G, class Traits = graph_traits<G>>
constexpr bool has_graph_property_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_graph_proprety);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_node_property_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_node_property);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_edge_property_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_edge_property);

template<class G, class Traits = graph_traits<G>>
constexpr bool is_user_defined_node_type_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(user_defined_node_type);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_node_container_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_node_container);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_adjacency_container_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_adjacency_container);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_edge_list_container_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_edge_list_container);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_edge_container_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_edge_container);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_in_adjacency_container_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_in_adjacency_container);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_in_edges_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(has_in_edges);

template<class G, class Traits = graph_traits<G>>
constexpr std::size_t max_node_size_v = ...;

template<class G, class Traits = graph_traits<G>>
constexpr std::size_t max_edge_size_v = ...;

template<class G, class Traits = graph_traits<G>>
constexpr bool has_invalid_node_v = ...;

template<class G, class Traits = graph_traits<G>>
constexpr node_t<G, Traits> invalid_node_v = ...;

template<class G, class Traits = graph_traits<G>>
constexpr bool is_user_defined_edge_type_v = ...;


template<class G, class Traits = graph_traits<G>>
constexpr bool parallel_edges_v = ...;

template<class G, class Traits = graph_traits<G>>
constexpr bool directed_edges_v = ...;

template<class G, class Traits = graph_traits<G>>
constexpr bool compressed_edges_v = BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER(compressed_edges);

template<class G, class Traits = graph_traits<G>>
using graph_property_t = ...;

template<class G, class Traits = graph_traits<G>>
using node_property_t = ...;

template<class G, class Traits = graph_traits<G>>
using edge_property_t = ...;

template<class G, class Traits = graph_traits<G>>
using node_t = ...;

template<class G, class Traits = graph_traits<G>>
using edge_t = ...;

template<class G, class Traits = graph_traits<G>>
using edge_repr_t = ...;
 
template<class G, class Traits = graph_traits<G>>
using node_container_t = ...;

template<class G, class Traits = graph_traits<G>>
using adjacency_container_t = ...;

template<class G, class Traits = graph_traits<G>>
using edge_list_container_t = ...;

template<class G, class Traits = graph_traits<G>>
using edge_container_t = ...;

template<class G, class Traits = graph_traits<G>>
using in_adjacency_container_t = ...;
*/
#undef BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER
} // namespace bxlx::graph::traits

#endif //BXLX_GRAPH_TRAITS_HPP
