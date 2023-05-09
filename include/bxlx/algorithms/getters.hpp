//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_GETTERS_HPP
#define BXLX_GRAPH_GETTERS_HPP

#include "constants.hpp"

namespace bxlx::graph {
namespace detail {
  using bxlx::graph::type_traits::detail::copy_cvref_t;
}

template<class G, class Traits = graph_traits<G>>
constexpr auto nodes(G&& graph)
      -> detail::copy_cvref_t<G&&, node_container_t<G, Traits>>;

template<class G, class Traits = graph_traits<G>>
constexpr auto edges(G&& graph)
      -> detail::copy_cvref_t<G&&, edge_container_t<G, Traits>>; // (2)

template<class G, class Traits = graph_traits<G>>
constexpr auto edge_list(G&& graph)
      -> detail::copy_cvref_t<G&&, edge_list_container_t<G, Traits>>; // (3)

template<class G, class Traits = graph_traits<G>>
constexpr auto adjacents(G&& graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G&&, adjacency_container_t<G, Traits>>; // (4)

template<class G, class Traits = graph_traits<G>>
constexpr auto adjacents(G* graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G, adjacency_container_t<G, Traits>> *; // (5)

template<class G, class Traits = graph_traits<G>>
constexpr auto in_adjacents(G&& graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G&&, in_adjacency_container_t<G, Traits>>; // (6)

template<class G, class Traits = graph_traits<G>>
constexpr auto in_adjacents(G* graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G, in_adjacency_container_t<G, Traits>> *; // (7)


template<class G, class Traits = graph_traits<G>>
constexpr auto invalid_edge(G const& graph)
      -> edge_repr_t<G, Traits>; // (0)

template<class G, class Traits = graph_traits<G>>
constexpr auto get_edge(G const& graph,
                        node_t<G, Traits> const& from,
                        node_t<G, Traits> const& to)
      -> edge_repr_t<G, Traits>; // (1)

template<class G, class Traits = graph_traits<G>>
constexpr auto get_edge(G const& graph, edge_t<G, Traits> const& edge)
      -> edge_repr_t<G, Traits>; // (2)

template<class G, class Traits = graph_traits<G>>
constexpr auto equal_edges(G const& graph,
                           node_t<G, Traits> const& from,
                           node_t<G, Traits> const& to)
      -> std::pair<edge_repr_t<G, Traits>, edge_repr_t<G, Traits>>; // (3)

template<class G, class Traits = graph_traits<G>>
constexpr auto equal_edges(G const& graph, edge_t<G, Traits> const& edge)
      -> std::pair<edge_repr_t<G, Traits>, edge_repr_t<G, Traits>>; // (4)

template<class G, class Traits = graph_traits<G>>
constexpr auto get_adjacency(G const& graph,
                             node_t<G, Traits> const& from,
                             node_t<G, Traits> const& to)
      -> edge_repr_t<G, Traits>; // (5)


template<class G, class Traits = graph_traits<G>>
constexpr auto graph_property(G&& graph)
      -> detail::copy_cvref_t<G&&, graph_property_t<G, Traits>>; // (1)

template<class G, class Traits = graph_traits<G>>
constexpr auto node_property(G&& graph, node_t<G, Traits>)
      -> detail::copy_cvref_t<G&&, node_property_t<G, Traits>>; // (2)

template<class G, class Traits = graph_traits<G>>
constexpr auto node_property(G* graph, node_t<G, Traits>)
      -> detail::copy_cvref_t<G, node_property_t<G, Traits>>*; // (3)

template<class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G&& graph,
                             node_t<G, Traits> const& from,
                             node_t<G, Traits> const& to)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>>; // (4)

template<class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G* graph,
                             node_t<G, Traits> const& from,
                             node_t<G, Traits> const& to)
      -> detail::copy_cvref_t<G, edge_property_t<G, Traits>>*; // (5)

template<class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G&& graph, edge_t<G, Traits> const& edge)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>>; // (6)

template<class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G* graph, edge_t<G, Traits> const& edge)
      -> detail::copy_cvref_t<G, edge_property_t<G, Traits>>*; // (7)

template<class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G&& graph, edge_repr_t<G, Traits>)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>>; // (8)
}

#endif //BXLX_GRAPH_GETTERS_HPP
