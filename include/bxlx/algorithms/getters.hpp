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
      if constexpr (type_traits::is_tuple_v<std::remove_reference_t<T>>) {
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
                           std::conditional_t<has_in_edges_v<G, Traits>,
                                              last_getter_t<1 + has_node_property_v<G, Traits>>,
                                              first_getter_t>,
                           noop_t>;

  template <class G, class Traits>
  using in_adjacent_container_getter =
        std::conditional_t<has_in_adjacency_container_v<G, Traits>, first_getter_t, noop_t>;

  template <class G, class Traits>
  using node_property_getter = std::conditional_t<has_node_property_v<G, Traits>, last_getter_t<>, noop_t>;

  template <class G, class Traits>
  using edge_property_getter = std::conditional_t<has_edge_property_v<G, Traits>, composition_t<last_getter_t<>, indirect_t>, noop_t>;

  template <class G, class Traits>
  using edge_index_getter = std::conditional_t<has_edge_container_v<G, Traits>, composition_t<last_getter_t<>, indirect_t>, noop_t>;

  template <class G, class Traits>
  using source_getter = std::conditional_t<has_edge_list_container_v<G, Traits>, composition_t<first_getter_t, indirect_t>, noop_t>;

  template <class G, class Traits>
  using target_getter = std::conditional_t<has_edge_list_container_v<G, Traits>, composition_t<last_getter_t<1 + has_edge_property_v<G, Traits>>, indirect_t>, noop_t>;
}

template <class G, class Traits = graph_traits<G>>
constexpr auto nodes(G&& graph) -> detail::copy_cvref_t<G&&, node_container_t<G, Traits>> {
  return std::forward<detail::copy_cvref_t<G&&, node_container_t<G, Traits>>>(
        detail::node_container_getter<G, Traits>{}(std::forward<G&&>(graph)));
}

template <class G, class Traits = graph_traits<G>>
constexpr auto edges(G&& graph) -> detail::copy_cvref_t<G&&, edge_container_t<G, Traits>> {
  return detail::edge_container_getter<G, Traits>{}(std::forward<G&&>(graph));
}

template <class G, class Traits = graph_traits<G>>
constexpr auto edge_list(G&& graph) -> detail::copy_cvref_t<G&&, edge_list_container_t<G, Traits>> {
  return detail::edge_list_container_getter<G, Traits>{}(std::forward<G&&>(graph));
}

namespace detail {
  template <class G, class Traits, class getter>
  constexpr decltype(auto) get_node_properties(G&& graph, node_t<G, Traits> const& node) {
    auto&& node_cont = nodes(std::forward<G&&>(graph));
    if constexpr (is_user_defined_node_type_v<G, Traits>) {
      if (auto [from, to] = node_cont.equal_range(node); from != to)
        return getter{}(std::get<0>(*from));
    } else {
      if (std::size(node_cont) > node) {
        return getter{}(*std::next(std::begin(node_cont), node));
      }
    }
    detail::throw_or_terminate<std::out_of_range>("Cannot find node");
  }

  template <class G, class Traits, class getter, class Res>
  constexpr auto get_node_properties_ptr(G* graph, node_t<G, Traits> const& node) -> Res {
    auto&& node_cont = nodes(*graph);
    if constexpr (is_user_defined_node_type_v<G, Traits>) {
      if (auto [from, to] = node_cont.equal_range(node); from != to)
        return std::addressof(getter{}(std::get<0>(*from)));
    } else {
      if (std::size(node_cont) > node)
        return std::addressof(getter{}(*std::next(std::begin(node_cont), node)));
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


template <class G, class Traits = graph_traits<G>>
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

template <class G, class Traits = graph_traits<G>>
constexpr auto
get_edge(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to) -> edge_repr_t<G, Traits> {
  if constexpr (has_adjacency_container_v<G, Traits>) {
    if (auto* adj = adjacents(&graph, from)) {
      if (auto [f, t] = adj->equal_range(to); f != t) {
        if constexpr (has_edge_container_v<G, Traits>) {
          return get_edge(graph, detail::edge_index_getter<G, Traits>{}(f));
        } else {
          return f;
        }
      }
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

template <class G, class Traits = graph_traits<G>>
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


template <class G, class Traits = graph_traits<G>>
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
}

#endif //BXLX_GRAPH_GETTERS_HPP
