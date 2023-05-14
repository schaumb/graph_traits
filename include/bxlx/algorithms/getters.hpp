//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_GETTERS_HPP
#define BXLX_GRAPH_GETTERS_HPP

#include "constants.hpp"
#include <climits>
#include <set>

namespace bxlx::graph {
namespace detail {
  using bxlx::graph::type_traits::detail::copy_cvref_t;
  struct noop_t {
    template <class... Ts>
    constexpr inline void operator()(Ts&&...) const noexcept {}
  };

  template <std::size_t I>
  struct tuple_getter_t {
    template <class T, class... Ts>
    [[nodiscard]] constexpr inline auto
    operator()(T&& val, Ts&&...) const noexcept -> copy_cvref_t<T&&, std::tuple_element_t<I, T>> {
      return std::get<I>(val);
    }
  };

  struct first_getter_t {
    template <class T, class... Ts>
    [[nodiscard]] constexpr inline decltype(auto) operator()(T&& val, Ts&&...) const noexcept {
      if constexpr (classification::classify<std::remove_reference_t<T>> == classification::type::tuple_like) {
        return first_getter_t{}(std::get<0>(std::forward<T&&>(val)));
      } else {
        return std::forward<T&&>(val);
      }
    }
  };

  template <std::size_t I = 1>
  struct last_getter_t {
    template <class T, class... Ts>
    [[nodiscard]] constexpr inline decltype(auto) operator()(T&& val, Ts&&...) const noexcept {
      if constexpr (!type_traits::is_tuple_v<std::remove_reference_t<T>>) {
        return std::forward<T&&>(val);
      } else if constexpr (std::tuple_size_v<std::remove_reference_t<T>> > I) {
        return last_getter_t<>{}(std::get<std::tuple_size_v<std::remove_reference_t<T>> - I>(std::forward<T&&>(val)));
      } else {
        return last_getter_t<I + 1 - std::tuple_size_v<T>>{}(std::get<0>(std::forward<T&&>(val)));
      }
    }
  };

  template <std::size_t I = 1>
  struct drop_last_t {};

  struct indirect_t {
    template <class T, class... Ts>
    [[nodiscard]] constexpr inline auto operator()(T&& val, Ts&&...) const noexcept -> decltype(*val) {
      return *val;
    }
  };

  struct identity_t {
    template <class T, class... Ts>
    [[nodiscard]] constexpr inline T&& operator()(T&& val, Ts&&...) const noexcept {
      return std::forward<T>(val);
    }
  };

  template <class type1, class type2>
  struct composition_t {
    template <class T, class... Ts>
    [[nodiscard]] constexpr inline auto operator()(T&& val, Ts&&... ts) const noexcept
          -> std::invoke_result_t<type1, std::invoke_result_t<type2, T&&, Ts&&...>, Ts&&...> {
      return type1{}(type2{}(std::forward<T>(val), ts...), ts...);
    }
  };


  template <class G, class Traits>
  using graph_property_getter = std::conditional_t<has_graph_property_v<G, Traits>, last_getter_t<>, noop_t>;

  template <class G, class Traits>
  using edge_container_getter = std::conditional_t<has_edge_container_v<G, Traits> || has_graph_property_v<G, Traits>,
                                                   last_getter_t<1 + has_graph_property_v<G, Traits>>,
                                                   noop_t>;
  template <class G, class Traits>
  using node_container_getter = std::conditional_t<has_node_container_v<G, Traits>, first_getter_t, noop_t>;

  template <class G, class Traits>
  using edge_list_container_getter = std::conditional_t<
        has_edge_list_container_v<G, Traits>,
        std::conditional_t<has_node_container_v<G, Traits>,
                           last_getter_t<1 + has_graph_property_v<G, Traits> + has_edge_container_v<G, Traits>>,
                           first_getter_t>,
        noop_t>;

  template <class G, class Traits>
  using adjacent_container_getter =
        std::conditional_t<has_adjacency_container_v<G, Traits>,
              std::conditional_t<has_node_container_v<G, Traits>,
                           composition_t<std::conditional_t<has_in_edges_v<G, Traits>,
                                              last_getter_t<1 + has_node_property_v<G, Traits>>,
                                              first_getter_t>, indirect_t>,
                                              first_getter_t>,
                           noop_t>;

  template <class G, class Traits>
  using in_adjacent_container_getter =
        std::conditional_t<has_in_adjacency_container_v<G, Traits>, composition_t<first_getter_t, indirect_t>, noop_t>;

  template <class G, class Traits>
  using node_property_getter = std::conditional_t<has_node_property_v<G, Traits>, composition_t<last_getter_t<>, indirect_t>, noop_t>;

  template <class G, class Traits>
  using edge_property_getter = std::conditional_t<has_edge_property_v<G, Traits>, composition_t<last_getter_t<>, indirect_t>, noop_t>;

  template <class G, class Traits>
  using edge_index_getter = std::conditional_t<has_edge_container_v<G, Traits>, composition_t<last_getter_t<>, indirect_t>, noop_t>;

  template <class G, class Traits>
  using source_getter = std::conditional_t<has_edge_list_container_v<G, Traits>, composition_t<first_getter_t, indirect_t>, noop_t>;

  template <class G, class Traits>
  using target_getter = std::conditional_t<has_edge_list_container_v<G, Traits>, composition_t<last_getter_t<1 + has_edge_property_v<G, Traits>>, indirect_t>, noop_t>;
}

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto nodes(G&& graph) -> detail::copy_cvref_t<G&&, node_container_t<G, Traits>> {
  return detail::node_container_getter<G, Traits>{}(std::forward<G&&>(graph));
}

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto edges(G&& graph) -> detail::copy_cvref_t<G&&, edge_container_t<G, Traits>> {
  return detail::edge_container_getter<G, Traits>{}(std::forward<G&&>(graph));
}

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
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

template <class G, class Traits = graph_traits<G>>
constexpr auto adjacents(G&& graph, node_t<G, Traits, true> const& node)
      -> detail::copy_cvref_t<G&&, adjacency_container_t<G, Traits>> {
  return detail::get_node_properties<G, Traits, detail::adjacent_container_getter<G, Traits>>(std::forward<G>(graph),
                                                                                              node);
}

template <class G, class Traits = graph_traits<G>>
constexpr auto
adjacents(G* graph, node_t<G, Traits> const& node) -> detail::copy_cvref_t<G, adjacency_container_t<G, Traits>>* {
  return detail::get_node_properties_ptr<G, Traits, detail::adjacent_container_getter<G, Traits>,
                                         detail::copy_cvref_t<G, adjacency_container_t<G, Traits>>*>(graph, node);
}

template <class G, class Traits = graph_traits<G>>
constexpr auto in_adjacents(G&& graph, node_t<G, Traits, true> const& node)
      -> detail::copy_cvref_t<G&&, in_adjacency_container_t<G, Traits>> {
  return detail::get_node_properties<G, Traits, detail::in_adjacent_container_getter<G, Traits>>(std::forward<G>(graph),
                                                                                                 node);
}

template <class G, class Traits = graph_traits<G>>
constexpr auto
in_adjacents(G* graph, node_t<G, Traits> const& node) -> detail::copy_cvref_t<G, in_adjacency_container_t<G, Traits>>* {
  return detail::get_node_properties_ptr<G, Traits, detail::in_adjacent_container_getter<G, Traits>,
                                         detail::copy_cvref_t<G, in_adjacency_container_t<G, Traits>>*>(graph, node);
}


template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto invalid_edge(G const&) -> edge_repr_t<G, Traits> {
  return {};
}

template <class G, class Traits = graph_traits<G>>
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

template<class G, class Traits = graph_traits<G>>
constexpr std::enable_if_t<has_node_container_v<G, Traits> || has_adjacency_container_v<G, Traits>, std::size_t> node_count(G const& graph) {
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

template<class Cmp = std::equal_to<>, class G, class Traits = graph_traits<G>>
constexpr std::enable_if_t<!has_node_container_v<G, Traits> && !has_adjacency_container_v<G, Traits>, std::size_t> node_count(G const& graph, Cmp&& cmp = {}) {
  std::size_t res {};
  auto&& edge_list_cont = edge_list(graph);
  const auto end = std::end(edge_list_cont),
             begin = std::begin(edge_list_cont);
  constexpr auto s = detail::source_getter<G, Traits>{};
  constexpr auto t = detail::target_getter<G, Traits>{};

  for (auto it = begin; it != end; ++it) {
    auto&& source = s(it);
    auto&& target = t(it);

    bool need_count_source = !cmp(source, target);
    bool need_count_target = true;

    for (auto it2 = std::next(it); need_count_source && need_count_target && it2 != end; ++it2) {
      if (need_count_source)
        need_count_source = !cmp(source, s(it2)) && !cmp(source, t(it2));
      if (need_count_target)
        need_count_target = !cmp(target, s(it2)) && !cmp(target, t(it2));
    }

    res += need_count_source + need_count_target;
  }

  return res;
}

template <class G, class Traits = graph_traits<G>>
constexpr auto
get_edge(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to) -> edge_repr_t<G, Traits> {
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

template <class G, class Traits = graph_traits<G>>
constexpr auto equal_edges(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> std::pair<edge_repr_t<G, Traits>, edge_repr_t<G, Traits>>; // (3)

template <class G, class Traits = graph_traits<G>>
constexpr auto equal_edges(G& graph, edge_t<G, Traits> const& edge)
      -> std::pair<edge_repr_t<G, Traits>, edge_repr_t<G, Traits>>; // (4)

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
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


template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto graph_property(G&& graph) -> detail::copy_cvref_t<G&&, graph_property_t<G, Traits>> {
  return detail::graph_property_getter<G, Traits>{}(std::forward<G&&>(graph));
}

template <class G, class Traits = graph_traits<G>>
constexpr auto
node_property(G&& graph, node_t<G, Traits, true> const& node) -> detail::copy_cvref_t<G&&, node_property_t<G, Traits>> {
  return detail::get_node_properties<G, Traits, detail::node_property_getter<G, Traits>>(std::forward<G>(graph), node);
}

template <class G, class Traits = graph_traits<G>>
constexpr auto
node_property(G* graph, node_t<G, Traits> const& node) -> detail::copy_cvref_t<G, node_property_t<G, Traits>>* {
  return detail::get_node_properties_ptr<G, Traits, detail::node_property_getter<G, Traits>,
                                         detail::copy_cvref_t<G, node_property_t<G, Traits>>*>(graph, node);
}

template <class G, class Traits = graph_traits<G>>
constexpr auto
edge_property(G&&, edge_repr_t<G, Traits, true> repr) -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>> {
  return detail::edge_property_getter<G, Traits>{}(repr);
}

template <class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G&& graph, node_t<G, Traits, true> const& from, node_t<G, Traits, true> const& to)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>> {
  if (auto&& e = get_edge(graph, from, to); e != invalid_edge(graph))
    return edge_property(std::forward<G>(graph), e);
  detail::throw_or_terminate<std::out_of_range>("edge not exists");
}

template <class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G* graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> detail::copy_cvref_t<G, edge_property_t<G, Traits>>* {
  if (auto&& e = get_edge(*graph, from, to); e != invalid_edge(*graph))
    return std::addressof(edge_property(*graph, e));
  return nullptr;
}

template <class G, class Traits = graph_traits<G>>
constexpr auto
edge_property(G&& graph, edge_t<G, Traits, true> const& edge) -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>> {
  if (auto&& e = get_edge(graph, edge); e != invalid_edge(graph))
    return edge_property(std::forward<G>(graph), e);
  detail::throw_or_terminate<std::out_of_range>("edge not exists");
}
template <class G, class Traits = graph_traits<G>>
constexpr auto
edge_property(G* graph, edge_t<G, Traits> const& edge) -> detail::copy_cvref_t<G, edge_property_t<G, Traits>>* {
  if (auto&& e = get_edge(*graph, edge); e != invalid_edge(*graph))
    return std::addressof(edge_property(*graph, e));
  return nullptr;
}

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
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

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr std::size_t adjacency_count(G const& graph) {
  return edge_count(graph);
}

template<class G, class Traits = graph_traits<G>>
constexpr std::enable_if_t<has_node_container_v<G, Traits> || has_adjacency_container_v<G, Traits>, bool> has_node(G const& graph, node_t<G, Traits> const& node) {
  if constexpr (has_node_container_v<G, Traits>) {
    return detail::get_node_properties_ptr<const G, Traits, detail::identity_t, bool>(&graph, node);
  } else {
    return node_count(graph) > node;
  }
}

template<class Cmp = std::equal_to<>, class G, class Traits = graph_traits<G>>
constexpr std::enable_if_t<!has_node_container_v<G, Traits> && !has_adjacency_container_v<G, Traits>, bool> has_node(G const& graph, node_t<G, Traits> const& node, Cmp&& cmp = {}) {
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

template<class G, class Traits = graph_traits<G>>
constexpr bool has_edge(G const& graph, edge_t<G, Traits> const& edge) {
  return get_edge(graph, edge) != invalid_edge(graph);
}

template<class G, class Traits = graph_traits<G>>
constexpr bool has_edge(G const& graph,
                        node_t<G, Traits> const& from,
                        node_t<G, Traits> const& to) {
  return get_edge(graph, from, to) != invalid_edge(graph);
}

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
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

namespace detail {
  template<class G, class Traits, class = void>
  struct edge_iterable;

  template<class G, class Traits>
  struct edge_iterable<G, Traits, std::enable_if_t<!has_node_container_v<G, Traits> && has_adjacency_container_v<G, Traits>>> {
    G& g;
    node_t<G, Traits> start;

    struct const_iterator {
      using iterator_category = std::forward_iterator_tag;
      using value_type = std::pair<node_t<G, Traits>, edge_repr_t<G, Traits>>;
      using difference_type = std::ptrdiff_t;
      using pointer = const value_type*;
      using reference = const value_type&;

      iterator::bitset_iterator<adjacency_container_t<G, Traits>> it;
      node_t<G, Traits> start;

      constexpr bool operator!=(const_iterator const& rhs) const {
        return it != rhs.it;
      }

      constexpr const_iterator& operator++() {
        ++it;
        return *this;
      }
    };

    constexpr const_iterator begin() const {
      auto c = node_count(g);
      return {iterator::get_first_good(adjacent_container_getter<G, Traits>{}(g), start * c, (start+1)*c), start * c};
    }

    constexpr const_iterator end() const {
      return {};
    }

    constexpr std::size_t size() const {
      return std::distance(begin(), end());
    }
  };

  template<class G, class Traits>
  struct edge_iterable<G, Traits, std::enable_if_t<has_node_container_v<G, Traits> && has_adjacency_container_v<G, Traits>>> {
    G& g;
    node_t<G, Traits> start;

    struct const_iterator {
      using iterator_category = std::forward_iterator_tag;
      using value_type = std::pair<node_t<G, Traits>, edge_repr_t<G, Traits>>;
      using difference_type = std::ptrdiff_t;
      using pointer = const value_type*;
      using reference = const value_type&;

      using wrapper_it = std::conditional_t<std::is_const_v<G>,
                                            typename adjacency_container_t<G, Traits>::const_iterator,
                                            typename adjacency_container_t<G, Traits>::iterator>;
      G* g;
      wrapper_it start, it;

      constexpr bool operator!=(const_iterator const& rhs) const {
        return it != rhs.it;
      }

      constexpr const_iterator& operator++() {
        ++it;
        return *this;
      }

      constexpr value_type operator*() const {
        edge_repr_t<G, Traits> edge_repr;
        if constexpr (has_edge_container_v<G, Traits>) {
          edge_repr = get_edge(g, edge_index_getter<G, Traits>{}(it));
        } else {
          edge_repr = it;
        }
        node_t<G, Traits> node;
        if constexpr (is_user_defined_node_type_v<G, Traits>) {
          node = composition_t<first_getter_t, indirect_t>{}(it);
        } else {
          node = std::distance(it, start);
        }
        return {node, edge_repr};
      }
    };

    constexpr const_iterator begin() const {
      if (auto has_adj = adjacents(&g, start)) {
        return {&g, std::begin(*has_adj), std::begin(*has_adj)};
      }
      return {};
    }

    constexpr const_iterator end() const {
      if (auto has_adj = adjacents(&g, start)) {
        return {&g, std::begin(*has_adj), std::end(*has_adj)};
      }
      return {};
    }

    constexpr std::size_t size() const {
      return std::distance(begin(), end());
    }
  };

  template<class G, class Traits, class Check, class Neigh>
  struct edge_list_iterable {
    G& g;
    node_t<G, Traits> start;

    struct const_iterator {
      using iterator_category = std::forward_iterator_tag;
      using value_type = std::pair<node_t<G, Traits>, edge_repr_t<G, Traits>>;
      using difference_type = std::ptrdiff_t;
      using pointer = const value_type*;
      using reference = const value_type&;

      using wrapper_it = std::conditional_t<std::is_const_v<G>,
                                            typename edge_list_container_t<G, Traits>::const_iterator,
                                            typename edge_list_container_t<G, Traits>::iterator>;
      const edge_list_iterable* that;
      wrapper_it it, end;

      constexpr const_iterator(const edge_list_iterable& that, wrapper_it it, wrapper_it end) : that(&that), it(it), end(end) {
        if (it != end && Check{}(it) != that.start)
          ++*this;
      }

      constexpr bool operator!=(const_iterator const& rhs) const {
        return it != rhs.it;
      }

      constexpr const_iterator& operator++() {
        while (++it != end) {
          if (Check{}(it) == that->start)
            break;
        }
        return *this;
      }

      constexpr value_type operator*() const {
        if constexpr (has_edge_container_v<G, Traits>) {
          return {Neigh{}(it), get_edge(that->g, edge_index_getter<G, Traits>{}(it))};
        } else {
          return {Neigh{}(it), it};
        }
      }
    };

    constexpr const_iterator begin() const {
      auto& el = edge_list_container_getter<G, Traits>{}(g);
      return {*this, std::begin(el), std::end(el)};
    }

    constexpr const_iterator end() const {
      auto it = std::end(edge_list_container_getter<G, Traits>{}(g));
      return {*this, it, it};
    }

    constexpr std::size_t size() const {
      return std::distance(begin(), end());
    }
  };

  template<class G, class Traits>
  struct edge_iterable<G, Traits, std::enable_if_t<has_edge_list_container_v<G, Traits>>>
        : edge_list_iterable<G, Traits, source_getter<G, Traits>, target_getter<G, Traits>> {
  };


  template<class G, class Traits, class = void>
  struct in_edge_iterable;

  template<class G, class Traits>
  struct in_edge_iterable<G, Traits, std::enable_if_t<has_edge_list_container_v<G, Traits>>>
        : edge_list_iterable<G, Traits, target_getter<G, Traits>, source_getter<G, Traits>> {
  };

  template<class G, class Traits, class = void>
  struct node_iterable;


  template<class G, class Traits>
  struct node_iterable<G, Traits, std::enable_if_t<has_node_container_v<G, Traits>>> {
    G& g;
    template<class G2 = G, class Traits2 = Traits, class = void>
    struct const_iterator {
      using iterator_category = std::forward_iterator_tag;
      using value_type = node_t<G, Traits>;
      using difference_type = std::ptrdiff_t;
      using pointer = const value_type*;
      using reference = const value_type&;
      value_type n;

      constexpr const_iterator(const G2& g, bool end) :
        n(end ? std::size(node_container_getter<G, Traits>{}(g)) : value_type{}) {}

      constexpr bool operator!=(const_iterator const& rhs) const {
        return n != rhs.n;
      }

      constexpr const_iterator& operator++() {
        ++n;
        return *this;
      }

      constexpr value_type operator*() const {
        return n;
      }
    };

    template<class G2, class Traits2>
    struct const_iterator<G2, Traits2, std::enable_if_t<is_associative<node_container_t, G2, Traits2>>> {
      using iterator_category = std::forward_iterator_tag;
      using value_type = node_t<G, Traits>;
      using difference_type = std::ptrdiff_t;
      using pointer = const value_type*;
      using reference = const value_type&;
      using It = typename node_container_t<G, Traits>::const_iterator;
      It it;

      constexpr const_iterator(const G2& g, bool end) :
        it (end ? std::end(node_container_getter<G, Traits>{}(g)) : std::begin(node_container_getter<G, Traits>{}(g))){}

      constexpr bool operator!=(const_iterator const& rhs) const {
        return it != rhs.it;
      }

      constexpr const_iterator& operator++() {
        ++it;
        return *this;
      }

      constexpr value_type operator*() const {
        return composition_t<first_getter_t, indirect_t>{}(it);
      }
    };



    constexpr const_iterator<> begin() const {
      return {g, false};
    }

    constexpr const_iterator<> end() const {
      return {g, true};
    }

    constexpr std::size_t size() const {
      return std::size(node_container_getter<G, Traits>{}(g));
    }
  };

  template<class T>
  constexpr T log2(T x) {
    return x == T{1} ? T{} : 1+log2(x >> 1);
  }

  template<class T>
  constexpr T log2_ceil(T x) {
    return x == T{1} ? T{} : log2(x - 1) + 1;
  }

  template<class G, class Traits = graph_traits<G>, class States = std::integral_constant<std::size_t, 2>, class = void>
  struct node_set;

  template<class G, class Traits, class States>
  struct node_set<G, Traits, States,
                  std::enable_if_t<max_node_size_v<G, Traits> != std::numeric_limits<std::size_t>::max()>> {
    constexpr static std::size_t BITS_PER_SET = sizeof(std::uint64_t) * CHAR_BIT;
    constexpr static std::size_t USED_BITS_PER_NODE = log2_ceil(States::value);
    constexpr static std::size_t BIT_MASK = (1 << USED_BITS_PER_NODE) - 1;
    std::array<std::uint64_t, (max_node_size_v<G, Traits> * USED_BITS_PER_NODE + (BITS_PER_SET - 1) ) / BITS_PER_SET> bitset {};

    using const_iterator = iterator::bitset_iterator<const node_set, iterator::good_bits, node_t<G, Traits>>;
    using iterator = iterator::bitset_iterator<node_set, iterator::all_bits, node_t<G, Traits>>;

    constexpr const_iterator begin() const {
      return iterator::get_first_good(*this);
    }

    constexpr const_iterator end() const {
      return {this, size(), size()};
    }

    struct reference {
      using ToType = std::conditional_t<USED_BITS_PER_NODE == 1, bool, std::size_t>;
      iterator it;

      constexpr operator ToType() const {
        auto ix = it.index;
        return (it.obj->bitset[ix / (BITS_PER_SET / USED_BITS_PER_NODE)] >> (ix % (BITS_PER_SET / USED_BITS_PER_NODE))) & BIT_MASK;
      }

      constexpr reference& operator=(std::conditional_t<USED_BITS_PER_NODE == 1, bool, std::size_t> res) {
        auto ix = it.index;
        it.obj->bitset[ix / (BITS_PER_SET / USED_BITS_PER_NODE)] ^= ((res & BIT_MASK) ^ static_cast<ToType>(*this)) << (ix % (BITS_PER_SET / USED_BITS_PER_NODE));
        return *this;
      }
    };

    constexpr reference operator[](node_t<G, Traits> const& n) {
      return {{this, n, static_cast<node_t<G, Traits>>(size())}};
    }

    constexpr std::size_t size() const noexcept {
      return max_node_size_v<G, Traits>;
    }

    using type = node_set<G, Traits, States>;
  };

  template<class G, class Traits, class States>
  struct node_set<G, Traits, States, std::enable_if_t<is_user_defined_node_type_v<G, Traits> && States{} == 2>> {
    using type = std::set<node_t<G, Traits>>;
  };

  template<class G, class Traits = graph_traits<G>, class States = std::integral_constant<std::size_t, 2>, bool = it_is_a_graph_v<G, Traits>>
  using node_set_t = typename node_set<G, Traits, States>::type;
}


template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto node_indices(G& graph)
      -> detail::node_iterable<G, Traits> {
  return {graph};
}

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto out_edges(G& graph, node_t<G, Traits> const& node)
      -> detail::edge_iterable<G, Traits> {
  return {graph, node};
}

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto in_edges(G& graph, node_t<G, Traits> const& node)
      -> detail::in_edge_iterable<G, Traits> {
  return {graph, node};
}
}

#endif //BXLX_GRAPH_GETTERS_HPP
