//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_GETTERS_HPP
#define BXLX_GRAPH_GETTERS_HPP

#include "constants.hpp"
#include "getter_types.hpp"
#include "bitset_iterator.hpp"
#include "node_iterator.hpp"
#include "edge_iterator.hpp"
#include "node_set.hpp"
#include <climits>
#include <set>

namespace bxlx::graph {

template <class G, class Traits, bool>
constexpr auto nodes(G&& graph) -> detail::copy_cvref_t<G&&, node_container_t<G, Traits>> {
  return detail::node_container_getter<G, Traits>{}(std::forward<G&&>(graph));
}

template <class G, class Traits, bool>
constexpr auto edges(G&& graph) -> detail::copy_cvref_t<G&&, edge_container_t<G, Traits>> {
  return detail::edge_container_getter<G, Traits>{}(std::forward<G&&>(graph));
}

template <class G, class Traits, bool>
constexpr auto edge_list(G&& graph) -> detail::copy_cvref_t<G&&, edge_list_container_t<G, Traits>> {
  return detail::edge_list_container_getter<G, Traits>{}(std::forward<G&&>(graph));
}

namespace detail {
  template <class G, class Traits, class getter>
  constexpr decltype(auto) get_node_properties(G&& graph, node_t<G, Traits> const& node) {
    auto&& node_cont = nodes(std::forward<G&&>(graph));
    if constexpr (is_user_defined_node_type_v<G, Traits>) {
      if (auto [from, to] = node_cont.equal_range(node); from != to)
        return getter{}(from);
    } else {
      if (std::size(node_cont) > node) {
        return getter{}(std::next(std::begin(node_cont), node));
      }
    }
    detail::throw_or_terminate<std::out_of_range>("Cannot find node");
  }

  template <class G, class Traits, class getter, class Res>
  constexpr auto get_node_properties_ptr(G* graph, node_t<G, Traits> const& node) -> Res {
    auto&& node_cont = nodes(*graph);
    if constexpr (is_user_defined_node_type_v<G, Traits>) {
      if (auto [from, to] = node_cont.equal_range(node); from != to)
        return std::addressof(getter{}(from));
    } else {
      if (std::size(node_cont) > node)
        return std::addressof(getter{}(std::next(std::begin(node_cont), node)));
    }
    return {};
  }
}

template <class G, class Traits>
constexpr auto adjacents(G&& graph, node_t<G, Traits, true> const& node)
      -> detail::copy_cvref_t<G&&, adjacency_container_t<G, Traits>> {
  return detail::get_node_properties<G, Traits, detail::adjacent_container_getter<G, Traits>>(std::forward<G>(graph),
                                                                                              node);
}

template <class G, class Traits>
constexpr auto adjacents(G* graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G, adjacency_container_t<G, Traits>>* {
  return detail::get_node_properties_ptr<G, Traits, detail::adjacent_container_getter<G, Traits>,
                                         detail::copy_cvref_t<G, adjacency_container_t<G, Traits>>*>(graph, node);
}

template <class G, class Traits>
constexpr auto in_adjacents(G&& graph, node_t<G, Traits, true> const& node)
      -> detail::copy_cvref_t<G&&, in_adjacency_container_t<G, Traits>> {
  return detail::get_node_properties<G, Traits, detail::in_adjacent_container_getter<G, Traits>>(std::forward<G>(graph),
                                                                                                 node);
}

template <class G, class Traits>
constexpr auto in_adjacents(G* graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G, in_adjacency_container_t<G, Traits>>* {
  return detail::get_node_properties_ptr<G, Traits, detail::in_adjacent_container_getter<G, Traits>,
                                         detail::copy_cvref_t<G, in_adjacency_container_t<G, Traits>>*>(graph, node);
}


template <class G, class Traits, bool>
constexpr auto invalid_edge(G const&) -> edge_repr_t<G, Traits> {
  return {};
}

template <class G, class Traits>
constexpr auto get_edge(G& graph, edge_t<G, Traits> const& edge) -> edge_repr_t<G, Traits> {
  if constexpr (has_edge_container_v<G, Traits>) {
    auto&& edge_cont = edges(graph);
    if constexpr (is_user_defined_edge_type_v<G, Traits>) {
      if (auto [from, to] = edge_cont.equal_range(edge); from != to)
        return from;
    } else {
      if (std::size(edge_cont) > edge)
        return std::next(std::begin(edge_cont), edge);
    }
  } else {
    auto&& edge_list_cont = edge_list(graph);
    if (auto [from, to] = edge_list_cont.equal_range(edge); from != to)
      return from;
  }
  return invalid_edge(graph);
}

template<class G, class Traits, bool>
constexpr auto node_count(G const& graph)
      -> std::enable_if_t<has_node_container_v<G, Traits> || has_adjacency_container_v<G, Traits>, std::size_t> {
  if constexpr (has_node_container_v<G, Traits>) {
    return std::size(detail::node_container_getter<G, Traits>{}(graph));
  } else {
    if (auto size = std::size(detail::adjacent_container_getter<G, Traits>{}(graph)); detail::is_square_num(size)) {
      return detail::constexpr_sqrt(size);
    } else if (detail::is_k_x_km1(size)) {
      return detail::constexpr_sqrt_no_self(size);
    } else if (detail::is_k_x_km1_d_2(size)) {
      return detail::constexpr_sqrt_no_self(size * 2) + 1;
    } else {
      throw;
    }
  }
}

template<class Cmp, class G, class Traits, bool>
constexpr auto node_count(G const& graph, Cmp&& cmp)
      -> std::enable_if_t<!has_node_container_v<G, Traits> && !has_adjacency_container_v<G, Traits>, std::size_t> {
  return std::size(node_indices(graph, std::forward<Cmp>(cmp)));
}

template <class G, class Traits>
constexpr auto get_edge(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> edge_repr_t<G, Traits> {
  if constexpr (has_adjacency_container_v<G, Traits>) {
    if constexpr (has_node_container_v<G, Traits>) {
      if (auto* adj = adjacents(&graph, from)) {
        if constexpr (detail::is_associative<node_container_t, G, Traits>) {
          if (auto [f, t] = adj->equal_range(to); f != t) {
            if constexpr (has_edge_container_v<G, Traits>) {
              return get_edge(graph, detail::edge_index_getter<G, Traits>{}(f));
            } else {
              return f;
            }
          }
        } else {
          auto it = std::begin(*adj), end = std::end(*adj);
          while (it != end) {
            if (detail::composition_t<detail::first_getter_t, detail::indirect_t>{}(it) == to) {
              if constexpr (has_edge_container_v<G, Traits>) {
                return get_edge(graph, detail::edge_index_getter<G, Traits>{}(it));
              } else {
                return it;
              }
            }
            ++it;
          }
        }
      }
    } else {
      auto&& adj_mat = detail::adjacent_container_getter<G, Traits>{}(graph);
      return iterator::bitset_iterator<adjacency_container_t<G, Traits>>{&adj_mat, from * node_count(graph) + to,
        std::size(adj_mat)};
    }
  } else {
    auto&& edge_list_cont = edge_list(graph);
    if constexpr (detail::is_associative<edge_list_container_t, G, Traits>) {
      if (auto [f, t] = edge_list_cont.equal_range({from, to}); f != t) {
        if constexpr (has_edge_container_v<G, Traits>) {
          return get_edge(graph, detail::edge_index_getter<G, Traits>{}(f));
        } else {
          return f;
        }
      }
    } else {
      const auto end = std::end(edge_list_cont);
      auto it = std::begin(edge_list_cont);
      while (it != end) {
        if (detail::source_getter<G, Traits>{}(it) == from &&
            detail::target_getter<G, Traits>{}(it) == to) {
          if constexpr (has_edge_container_v<G, Traits>) {
            return get_edge(graph, detail::edge_index_getter<G, Traits>{}(it));
          } else {
            return it;
          }
        }
        ++it;
      }
    }
  }
  return invalid_edge(graph);
}

template <class G, class Traits>
constexpr auto equal_edges(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> std::pair<edge_repr_t<G, Traits>, edge_repr_t<G, Traits>>; // (3)

template <class G, class Traits>
constexpr auto equal_edges(G& graph, edge_t<G, Traits> const& edge)
      -> std::pair<edge_repr_t<G, Traits>, edge_repr_t<G, Traits>>; // (4)

template <class G, class Traits, bool>
constexpr auto get_adjacency(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> edge_repr_t<G, Traits> {
  if constexpr (detail::has_directed_edges_v<G, Traits>) {
    if constexpr (!directed_edges_v<G, Traits>) {
      if (auto && edge = get_edge(graph, to, from); edge != invalid_edge(graph))
        return edge;
    }
  }
  return get_edge(graph, from, to);
}


template <class G, class Traits, bool>
constexpr auto graph_property(G&& graph) -> detail::copy_cvref_t<G&&, graph_property_t<G, Traits>> {
  return detail::graph_property_getter<G, Traits>{}(std::forward<G&&>(graph));
}

template <class G, class Traits>
constexpr auto node_property(G&& graph, node_t<G, Traits, true> const& node)
      -> detail::copy_cvref_t<G&&, node_property_t<G, Traits>> {
  return detail::get_node_properties<G, Traits, detail::node_property_getter<G, Traits>>(std::forward<G>(graph), node);
}

template <class G, class Traits>
constexpr auto node_property(G* graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G, node_property_t<G, Traits>>* {
  return detail::get_node_properties_ptr<G, Traits, detail::node_property_getter<G, Traits>,
                                         detail::copy_cvref_t<G, node_property_t<G, Traits>>*>(graph, node);
}

template <class G, class Traits>
constexpr auto edge_property(G&&, edge_repr_t<G, Traits, true> repr)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>> {
  return detail::edge_property_getter<G, Traits>{}(repr);
}

template <class G, class Traits>
constexpr auto edge_property(G&& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>> {
  if (auto&& e = get_edge(graph, from, to); e != invalid_edge(graph))
    return edge_property(std::forward<G>(graph), e);
  detail::throw_or_terminate<std::out_of_range>("edge not exists");
}

template <class G, class Traits>
constexpr auto edge_property(G* graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> detail::copy_cvref_t<G, edge_property_t<G, Traits>>* {
  if (auto&& e = get_edge(*graph, from, to); e != invalid_edge(*graph))
    return std::addressof(edge_property(*graph, e));
  return nullptr;
}

template <class G, class Traits>
constexpr auto edge_property(G&& graph, edge_t<G, Traits, true> const& edge)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>> {
  if (auto&& e = get_edge(graph, edge); e != invalid_edge(graph))
    return edge_property(std::forward<G>(graph), e);
  detail::throw_or_terminate<std::out_of_range>("edge not exists");
}
template <class G, class Traits>
constexpr auto edge_property(G* graph, edge_t<G, Traits> const& edge)
      -> detail::copy_cvref_t<G, edge_property_t<G, Traits>>* {
  if (auto&& e = get_edge(*graph, edge); e != invalid_edge(*graph))
    return std::addressof(edge_property(*graph, e));
  return nullptr;
}

template<class G, class Traits, bool>
constexpr std::size_t edge_count(G const& graph) {
  if constexpr (has_edge_container_v<G, Traits>) {
    return std::size(detail::edge_container_getter<G, Traits>{}(graph));
  } else if constexpr (has_edge_list_container_v<G, Traits>) {
    return std::size(detail::edge_list_container_getter<G, Traits>{}(graph));
  } else if constexpr (has_node_container_v<G, Traits>) {
    return 0;
  } else {
    auto first = iterator::get_first_good(detail::adjacent_container_getter<const G, Traits>{}(graph)),
         end = decltype(first){};
    return std::distance(first, end);
  }
}

template<class G, class Traits, bool>
constexpr std::size_t adjacency_count(G const& graph) {
  return edge_count(graph);
}

template<class G, class Traits>
constexpr auto has_node(G const& graph, node_t<G, Traits> const& node)
      -> std::enable_if_t<has_node_container_v<G, Traits> || has_adjacency_container_v<G, Traits>, bool> {
  if constexpr (has_node_container_v<G, Traits>) {
    return detail::get_node_properties_ptr<const G, Traits, detail::identity_t, bool>(&graph, node);
  } else {
    return node_count(graph) > node;
  }
}

template<class Cmp, class G, class Traits>
constexpr auto has_node(G const& graph, node_t<G, Traits> const& node, Cmp&& cmp)
      -> std::enable_if_t<!has_node_container_v<G, Traits> && !has_adjacency_container_v<G, Traits>, bool> {
  auto&& edge_list_cont = edge_list(graph);
  const auto end = std::end(edge_list_cont);
  auto it = std::begin(edge_list_cont);
  while (it != end) {
    if (cmp(detail::source_getter<G, Traits>{}(it), node) ||
        cmp(detail::target_getter<G, Traits>{}(it), node)) {
      return true;
    }
    ++it;
  }
  return false;
}

template<class G, class Traits>
constexpr bool has_edge(G const& graph, edge_t<G, Traits> const& edge) {
  return get_edge(graph, edge) != invalid_edge(graph);
}

template<class G, class Traits>
constexpr bool has_edge(G const& graph,
                        node_t<G, Traits> const& from,
                        node_t<G, Traits> const& to) {
  return get_edge(graph, from, to) != invalid_edge(graph);
}

template<class G, class Traits, bool>
constexpr bool has_adjacency(G const& graph,
                             node_t<G, Traits> const& from,
                             node_t<G, Traits> const& to) {
  if constexpr (detail::has_directed_edges_v<G, Traits>) {
    if constexpr (!directed_edges_v<G, Traits>) {
      if (auto && edge = has_edge(graph, to, from))
        return edge;
    }
  }
  return has_edge(graph, from, to);
}


template<class G, class Traits, bool>
constexpr auto node_indices(G& graph) -> std::enable_if_t<
      has_node_container_v<G, Traits> || has_adjacency_container_v<G, Traits>, iterator::node_iterable<G, Traits>> {
  return {graph};
}

template<class Cmp, class G, class Traits, bool>
constexpr auto node_indices(G& graph, Cmp&& cmp) -> std::enable_if_t<
      !has_node_container_v<G, Traits> && !has_adjacency_container_v<G, Traits>, iterator::node_iterable<G, Traits, Cmp>> {
  return {graph, std::forward<Cmp>(cmp)};
};


template<class G, class Traits, bool>
constexpr auto out_edges(G& graph, node_t<G, Traits> const& node)
      -> iterator::edge_iterable<G, Traits> {
  return {graph, node};
}

template<class G, class Traits, bool>
constexpr auto in_edges(G& graph, node_t<G, Traits> const& node)
      -> iterator::in_edge_iterable<G, Traits> {
  return {graph, node};
}
}

#endif //BXLX_GRAPH_GETTERS_HPP
