//
// Created by qqcs on 6/18/23.
//

#ifndef BXLX_GRAPH_GETTTER_TYPES_HPP
#define BXLX_GRAPH_GETTTER_TYPES_HPP

#include "constants.hpp"

namespace bxlx::graph::detail {
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

#endif //BXLX_GRAPH_GETTTER_TYPES_HPP
