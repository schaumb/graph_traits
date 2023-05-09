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
constexpr bool has_adjacency_container_v<G, Traits, true, std::void_t<typename Traits::adjacency_container::type>> = true;

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
constexpr bool has_in_adjacency_container_v<G, Traits, true, std::void_t<typename Traits::in_adjacency_container::type>> = true;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool has_in_edges_v = Traits::in_edges_t::value;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool compressed_edges_v = Traits::compressed_edges::type::value;


template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using graph_property_t = typename Traits::graph_property::type;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using node_property_t = typename Traits::node_property::type;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using edge_property_t = typename Traits::edge_property::type;


template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using node_t = typename Traits::node_type::type;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using edge_t = typename Traits::edge_type::type;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using node_container_t = typename Traits::node_container::type;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using adjacency_container_t = typename Traits::adjacency_container::type;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using edge_list_container_t = typename Traits::edge_list_container::type;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using edge_container_t = typename Traits::edge_container::type;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using in_adjacency_container_t = typename Traits::in_adjacency_container::type;

namespace detail {
  template<template<class, class, bool> class container_type, class G, class Traits, class = void>
  constexpr std::size_t get_constexpr_size = {};

  template<template<class, class, bool> class container_type, class G, class Traits>
  constexpr std::size_t get_constexpr_size<container_type, G, Traits,
                                    std::enable_if_t<classification::classify<container_type<G, Traits, true>> == classification::type::range ||
                                                     classification::classify<container_type<G, Traits, true>> == classification::type::map_like>> =
        type_traits::range_constexpr_size_v<container_type<G, Traits, true>>;

  template<template<class, class, bool> class container_type, class G, class Traits>
  constexpr std::size_t get_constexpr_size<container_type, G, Traits,
                                    std::enable_if_t<classification::classify<container_type<G, Traits, true>> == classification::type::bitset>> =
        type_traits::detail::constexpr_std_size_v<container_type<G, Traits, true>>;

  template<template<class, class, bool> class container_type, class G, class Traits>
  constexpr bool has_constexpr_size = get_constexpr_size<container_type, G, Traits> > 0;

  template<template<class, class, bool> class container_type, class G, class Traits, class = void>
  constexpr bool is_associative = false;
  template<template<class, class, bool> class container_type, class G, class Traits>
  constexpr bool is_associative<container_type, G, Traits, std::enable_if_t<
    type_traits::is_range_v<container_type<G, Traits, true>>>> =
        type_traits::range_type_v<container_type<G, Traits, true>> == type_traits::range_type_t::set_like ||
        type_traits::range_type_v<container_type<G, Traits, true>> == type_traits::range_type_t::map_like;


  template <class T>
  constexpr T sqrt_helper(T x, T lo, T hi) {
    if (lo == hi)
      return lo;
    else if (const T mid = (lo + hi + 1) / 2; x / mid < mid)
      return sqrt_helper<T>(x, lo, mid - 1);
    else
      return sqrt_helper(x, mid, hi);
  }

  template <class T>
  constexpr T constexpr_sqrt(T x) {
    return sqrt_helper<T>(x, T{}, x / 2 + 1);
  }

  template <class T>
  constexpr T constexpr_sqrt_no_self(T x) {
    // N^2 - N - x = 0;
    return (1 + constexpr_sqrt(1 + 4 * x)) / 2;
  }

  template <class T>
  constexpr bool is_square_num(T n) {
    const T sq = constexpr_sqrt(n);
    return sq * sq == n;
  }

  template <class T>
  constexpr bool is_k_x_km1(T n) {
    const T k = constexpr_sqrt_no_self(n);
    return k * (k-1) == n;
  }

  template <class T>
  constexpr bool is_k_x_km1_d_2(T n) {
    const T k = constexpr_sqrt_no_self(n*2);
    return k * (k-1) / 2 == n;
  }

  template<class, class = void>
  constexpr inline auto constexpr_number = [] { throw; };
  template<class T>
  constexpr inline auto constexpr_number<T, std::enable_if_t<type_traits::detail::is_constexpr_initializable_v<T>>>
        = [] { return ~T{}; };

  template<class T, bool = type_traits::is_defined_v<T>, class = void>
  constexpr inline bool is_constexpr_number_v {};
  template<class T>
  constexpr inline bool is_constexpr_number_v<T, true, std::void_t<decltype(~std::declval<T>())>> = type_traits::detail::is_constexpr<&constexpr_number<std::remove_cv_t<T>>>(0);

  template<class G, class Traits, class = void>
  struct edge_repr;

  template<class G, class Traits>
  struct edge_repr<G, Traits, std::enable_if_t<has_edge_container_v<G, Traits>>> {
    using type = typename edge_container_t<G, Traits>::const_iterator;
  };
  template<class G, class Traits>
  struct edge_repr<G, Traits, std::enable_if_t<!has_edge_container_v<G, Traits> && has_edge_list_container_v<G, Traits>>> {
    using type = typename edge_list_container_t<G, Traits>::const_iterator;
  };
  template<class G, class Traits>
  struct edge_repr<G, Traits, std::enable_if_t<!has_edge_container_v<G, Traits> && has_adjacency_container_v<G, Traits>>> {
    using type = typename adjacency_container_t<G, Traits>::const_iterator;
  };
}

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr std::size_t max_node_size_v = std::numeric_limits<std::size_t>::max();
template<class G, class Traits>
constexpr std::size_t max_node_size_v<G, Traits, true,
      std::enable_if_t<detail::has_constexpr_size<node_container_t, G, Traits>>> = detail::get_constexpr_size<node_container_t, G, Traits>;

template<class G, class Traits>
constexpr std::size_t max_node_size_v<G, Traits, true,
                                      std::enable_if_t<has_node_container_v<G, Traits> &&
                                            !detail::has_constexpr_size<node_container_t, G, Traits> &&
                                                       detail::has_constexpr_size<edge_list_container_t, G, Traits> &&
                                                       (representation_v<G, Traits, true> == representation_t::adjacency_matrix)>> =
      detail::get_constexpr_size<edge_list_container_t, G, Traits>;
template<class G, class Traits>
constexpr std::size_t max_node_size_v<G, Traits, true,
                                      std::enable_if_t<!has_node_container_v<G, Traits> &&
                                            !has_adjacency_container_v<G, Traits> &&
                                            detail::has_constexpr_size<edge_container_t, G, Traits>>> =
      detail::get_constexpr_size<edge_container_t, G, Traits> * 2;
template<class G, class Traits>
constexpr std::size_t max_node_size_v<G, Traits, true,
                                      std::enable_if_t<!has_node_container_v<G, Traits> &&
                                                       !detail::has_constexpr_size<edge_container_t, G, Traits> &&
                                                       detail::has_constexpr_size<edge_list_container_t, G, Traits>>> =
      detail::get_constexpr_size<edge_list_container_t, G, Traits> * 2;
template<class G, class Traits>
constexpr std::size_t max_node_size_v<G, Traits, true,
                                      std::enable_if_t<!has_node_container_v<G, Traits> &&
                                                       detail::has_constexpr_size<adjacency_container_t, G, Traits>>> =
      detail::is_square_num(detail::get_constexpr_size<adjacency_container_t, G, Traits>)
      ? detail::constexpr_sqrt(detail::get_constexpr_size<adjacency_container_t, G, Traits>) :
      detail::is_k_x_km1(detail::get_constexpr_size<adjacency_container_t, G, Traits>)
      ? detail::constexpr_sqrt_no_self(detail::get_constexpr_size<adjacency_container_t, G, Traits>) :
      detail::is_k_x_km1_d_2(detail::get_constexpr_size<adjacency_container_t, G, Traits>)
      ? detail::constexpr_sqrt_no_self(detail::get_constexpr_size<adjacency_container_t, G, Traits> * 2) + 1 :
                  [] () -> std::size_t { throw; }();

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr std::size_t max_edge_size_v = std::numeric_limits<std::size_t>::max();
template<class G, class Traits>
constexpr std::size_t max_edge_size_v<G, Traits, true,
                                      std::enable_if_t<detail::has_constexpr_size<edge_container_t, G, Traits>>> =
      detail::get_constexpr_size<edge_container_t, G, Traits>;

template<class G, class Traits>
constexpr std::size_t max_edge_size_v<G, Traits, true,
                                      std::enable_if_t<!detail::has_constexpr_size<edge_container_t, G, Traits> &&
                                                       detail::has_constexpr_size<edge_list_container_t, G, Traits>
                                            >> = detail::get_constexpr_size<edge_list_container_t, G, Traits>;
template<class G, class Traits>
constexpr std::size_t max_edge_size_v<G, Traits, true,
                                      std::enable_if_t<!detail::has_constexpr_size<edge_container_t, G, Traits> &&
                                                       detail::has_constexpr_size<node_container_t, G, Traits> &&
                                                       detail::has_constexpr_size<adjacency_container_t, G, Traits> &&
                                                       (representation_v<G, Traits, true> == representation_t::adjacency_list)
                                                       >> =
      detail::get_constexpr_size<node_container_t, G, Traits> * detail::get_constexpr_size<adjacency_container_t, G, Traits>;
template<class G, class Traits>
constexpr std::size_t max_edge_size_v<G, Traits, true,
                                      std::enable_if_t<!has_node_container_v<G, Traits> &&
                                                       detail::has_constexpr_size<adjacency_container_t, G, Traits>
                                                       >> =
      detail::get_constexpr_size<adjacency_container_t, G, Traits>;

template<class G, class Traits>
constexpr std::size_t max_edge_size_v<G, Traits, true,
                                      std::enable_if_t<!detail::has_constexpr_size<edge_container_t, G, Traits> &&
                                                        has_node_container_v<G, Traits> &&
                                                       (detail::has_constexpr_size<node_container_t, G, Traits> ||
                                                       detail::has_constexpr_size<adjacency_container_t, G, Traits>) &&
                                                       (representation_v<G, Traits> == representation_t::adjacency_matrix)
                                                       >> =
      max_node_size_v<G, Traits> * max_node_size_v<G, Traits>;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool has_invalid_node_v = representation_v<G, Traits> != representation_t::adjacency_matrix &&
  (detail::has_constexpr_size<adjacency_container_t, G, Traits> || detail::has_constexpr_size<edge_list_container_t, G, Traits>);

template<class G, class Traits = graph_traits<G>, bool = has_invalid_node_v<G, Traits>, class = void>
constexpr auto invalid_node_v = [] { throw; } ();

template<class G, class Traits>
constexpr node_t<G, Traits> invalid_node_v<G, Traits, true, std::enable_if_t<detail::is_constexpr_number_v<node_t<G, Traits>>>> =
      detail::constexpr_number<node_t<G, Traits>>();

template<class G, class Traits>
constexpr node_t<G, Traits> invalid_node_v<G, Traits, true, std::enable_if_t<!detail::is_constexpr_number_v<node_t<G, Traits>> &&
                                                                             type_traits::detail::is_constexpr_initializable_v<node_t<G, Traits>>>> =
      type_traits::detail::constexpr_initializable<node_t<G, Traits>>();


template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool parallel_edges_v = [] () -> bool { throw; }();

template<class G, class Traits>
constexpr bool parallel_edges_v<G, Traits, true, std::enable_if_t<detail::is_associative<adjacency_container_t, G, Traits>>>
      = bxlx::graph::type_traits::is_associative_multi_v<adjacency_container_t<G, Traits>>;

template<class G, class Traits>
constexpr bool parallel_edges_v<G, Traits, true, std::enable_if_t<detail::is_associative<edge_list_container_t, G, Traits>>>
      = bxlx::graph::type_traits::is_associative_multi_v<edge_list_container_t<G, Traits>>;

template<class G, class Traits>
constexpr bool parallel_edges_v<G, Traits, true, std::void_t<typename Traits::multi_edge::type>>
      = Traits::multi_edge::type::value;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>, class = void>
constexpr bool directed_edges_v = [] () -> bool { throw; }();

template<class G, class Traits>
constexpr bool directed_edges_v<G, Traits, true, std::enable_if_t<has_edge_property_v<G, Traits, true> || has_in_edges_v<G, Traits, true>>> =
      !has_edge_container_v<G, Traits, true> || has_in_edges_v<G, Traits, true>;

template<class G, class Traits>
constexpr bool directed_edges_v<G, Traits, true, std::enable_if_t<!has_node_container_v<G, Traits> &&
                                                                  detail::has_constexpr_size<adjacency_container_t, G, Traits>>> =
      detail::is_square_num(detail::get_constexpr_size<adjacency_container_t, G, Traits>) ||
            detail::is_k_x_km1(detail::get_constexpr_size<adjacency_container_t, G, Traits>);

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
using edge_repr_t = typename detail::edge_repr<G, Traits>::type;

}

#endif //BXLX_GRAPH_CONSTANTS_HPP
