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

template <class G, class V = decltype(state_machine::graph::template valid<G>()), bool = V{}.value>
struct graph_traits {
  using reason_t = V;
};

template <class G, class V>
struct graph_traits<G, V, true> {
  using properties_t = typename V::properties;

  constexpr static representation_t representation =
        properties::has_property_v<properties_t, state_machine::adj_list>
              ? representation_t::adjacency_list :
        properties::has_property_v<properties_t, state_machine::adj_mat>
              ? representation_t::adjacency_matrix :
        properties::has_property_v<properties_t, state_machine::edge_list>
              ? representation_t::edge_list :
        representation_t{};

  static_assert(representation != representation_t{});

  using graph_property = properties::get_value<properties_t, state_machine::graph_prop_t>;
  using edge_property = properties::get_value<properties_t, state_machine::edge_prop_t>;
  using node_property = properties::get_value<properties_t, state_machine::node_prop_t>;

  using user_defined_node_t = properties::get_value_t<properties_t, state_machine::user_node_t, std::enable_if<true, std::false_type>>;
  using user_defined_edge_t = properties::get_value_t<properties_t, state_machine::user_edge_t>;

  using node_type = properties::get_value<properties_t, state_machine::node_type_t, std::enable_if<true, std::size_t>>;
  using edge_type = properties::get_value<properties_t, state_machine::edge_type_t>;

  using node_container = properties::get_value<properties_t, state_machine::node_container<next_type::Dummy>>;
  using edge_container = properties::get_value<properties_t, state_machine::edge_container<next_type::Dummy>>;

  using edge_list_container = properties::get_value<properties_t, state_machine::edge_list_n<next_type::Dummy>>;

  using adjacency_container =
        properties::get_value<properties_t, state_machine::indexed_type<state_machine::adj_list_cont<next_type::Dummy>, 1>,
        properties::get_value<properties_t, state_machine::indexed_type<state_machine::adj_mat_cont<next_type::Dummy>, 1>,
        properties::get_value<properties_t, state_machine::adj_list_cont<next_type::Dummy>,
        properties::get_value<properties_t, state_machine::adj_mat_cont<next_type::Dummy>>>>>;

  using in_adjacency_container = std::conditional_t<
        properties::has_property_v<properties_t, state_machine::indexed_type<state_machine::adj_list_cont<next_type::Dummy>, 1>> ||
        properties::has_property_v<properties_t, state_machine::indexed_type<state_machine::adj_mat_cont<next_type::Dummy>, 1>>,
        properties::get_value<properties_t, state_machine::adj_list_cont<next_type::Dummy>,
                              properties::get_value<properties_t, state_machine::adj_mat_cont<next_type::Dummy>>>,
        std::enable_if<false>
        >;

  using in_edges_t = properties::get_value_t<properties_t, state_machine::in_edges_t, std::enable_if<true, std::false_type>>;

  using multi_edge = properties::get_value<properties_t, state_machine::multi_edge_t>;
  using compressed_edges = properties::get_value<properties_t, state_machine::compressed_t>;
};

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
