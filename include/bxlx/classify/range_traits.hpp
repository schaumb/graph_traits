//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_RANGE_TRAITS_HPP
#define BXLX_GRAPH_RANGE_TRAITS_HPP

#include <array>
#include <string>
#include <string_view>

#include "type_traits.hpp"


#if defined(_MSC_VER) || defined(__APPLE__)
#  include <deque>
#  define BXLX_GRAPH_RANGE_TRAITS_DEQUE_NEEDED 1
#  include <regex>
#  define BXLX_GRAPH_RANGE_TRAITS_REGEX_NEEDED 1
#endif

#if (defined(__clang__) /* && __clang_major__ < 15 */) ||                                                              \
      (!defined(__clang__) && defined(__GNUC__) /* && __GNUC__ < 12 */)
#  include <unordered_set>
#  define BXLX_GRAPH_RANGE_TRAITS_UNORDERED_SET_NEEDED 1
#  include <unordered_map>
#  define BXLX_GRAPH_RANGE_TRAITS_UNORDERED_MAP_NEEDED 1
#endif

#ifdef __cpp_lib_ranges
#  include <ranges>
#  define BXLX_GRAPH_RANGE_TRAITS_RANGES_NEEDED 1
#endif

namespace bxlx::graph::type_classification::detail {

enum class storage_type_t {
  discontinuous,
  continuous /* has .data() */
};
enum class range_type_t {
  sequence,
  string_like, /* has .length() */
  queue_like,  /* has .push_front() and .push_back() */
  map_like,    /* has key_type, equal to value_type first component */
  set_like     /* has key_type, equal to value_type */
};

template <class T,
          class U = std::remove_cv_t<T>,
          class   = std::enable_if_t<std::is_same_v<T, U>>,
          class   = typename range_traits<T>::value_type>
using enable_if_is_range = T;

template <class T, bool any, bool any2>
struct range_traits<volatile enable_if_is_range<T>, any, any2> : range_traits<T> {};

template <class T, bool any, bool any2>
struct range_traits<const volatile enable_if_is_range<const T>, any, any2> : range_traits<const T> {};

template <class T, bool any>
struct range_traits<const enable_if_is_range<T>, any, true> : range_traits<T> {
  using value_type     = std::add_const_t<typename range_traits<T>::value_type>;
  using orig_reference = typename range_traits<T>::reference;
  using reference      = std::conditional_t<std::is_lvalue_reference_v<orig_reference>,
                                       std::add_lvalue_reference_t<value_type>,
                                       std::conditional_t<std::is_rvalue_reference_v<orig_reference>,
                                                          std::add_rvalue_reference_t<orig_reference>,
                                                          orig_reference>>;
};

template <bool any, class M, std::size_t S>
struct range_traits<std::array<M, S>, any, false /* it is a tuple */> {
  using reference    = M&;
  using value_type   = M;
  using iterator_tag = std::random_access_iterator_tag;

  constexpr static bool           defined = is_defined_v<M>;
  constexpr static storage_type_t storage = storage_type_t::continuous;
  constexpr static range_type_t   range   = range_type_t::sequence;
};

template <bool any, class CharT, class... Others>
struct range_traits<std::basic_string<CharT, Others...>, any, true> {
  using reference    = CharT&;
  using value_type   = CharT;
  using iterator_tag = std::random_access_iterator_tag;

  constexpr static bool           defined = is_defined_v<CharT>;
  constexpr static storage_type_t storage = storage_type_t::continuous;
  constexpr static range_type_t   range   = range_type_t::string_like;
};

template <bool any, class CharT, class... Others>
struct range_traits<std::basic_string_view<CharT, Others...>, any, true> {
  using reference    = CharT&;
  using value_type   = CharT;
  using iterator_tag = std::random_access_iterator_tag;

  constexpr static bool           defined = is_defined_v<CharT>;
  constexpr static storage_type_t storage = storage_type_t::continuous;
  constexpr static range_type_t   range   = range_type_t::string_like;
};

#ifdef BXLX_GRAPH_RANGE_TRAITS_DEQUE_NEEDED
#  undef BXLX_GRAPH_RANGE_TRAITS_DEQUE_NEEDED
template <bool any, class M, class... Others>
struct range_traits<std::deque<M, Others...>, any, true> {
  using reference    = M&;
  using value_type   = M;
  using iterator_tag = std::random_access_iterator_tag;

  constexpr static bool           defined = is_defined_v<M>;
  constexpr static storage_type_t storage = storage_type_t::discontinuous;
  constexpr static range_type_t   range   = range_type_t::queue_like;
};
#endif

#ifdef BXLX_GRAPH_RANGE_TRAITS_REGEX_NEEDED
#  undef BXLX_GRAPH_RANGE_TRAITS_REGEX_NEEDED
template <bool any, class M, class... Others>
struct range_traits<std::match_results<M, Others...>, any, true> {
  using reference    = const std::sub_match<M>&;
  using value_type   = std::sub_match<M>;
  using iterator_tag = std::random_access_iterator_tag;

  constexpr static bool           defined = is_defined_v<M>;
  constexpr static storage_type_t storage = storage_type_t::discontinuous;
  constexpr static range_type_t   range   = range_type_t::sequence;
};
#endif

#ifdef BXLX_GRAPH_RANGE_TRAITS_UNORDERED_SET_NEEDED
#  undef BXLX_GRAPH_RANGE_TRAITS_UNORDERED_SET_NEEDED
template <bool any, class M, class... Others>
struct range_traits<std::unordered_set<M, Others...>, any, true> {
  using reference    = const M&;
  using value_type   = const M;
  using iterator_tag = std::forward_iterator_tag;

  constexpr static bool           defined = is_defined_v<M>;
  constexpr static storage_type_t storage = storage_type_t::discontinuous;
  constexpr static range_type_t   range   = range_type_t::set_like;
};
template <bool any, class M, class... Others>
struct range_traits<std::unordered_multiset<M, Others...>, any, true> {
  using reference    = const M&;
  using value_type   = const M;
  using iterator_tag = std::forward_iterator_tag;

  constexpr static bool           defined = is_defined_v<M>;
  constexpr static storage_type_t storage = storage_type_t::discontinuous;
  constexpr static range_type_t   range   = range_type_t::set_like;
};
#endif

#ifdef BXLX_GRAPH_RANGE_TRAITS_UNORDERED_MAP_NEEDED
#  undef BXLX_GRAPH_RANGE_TRAITS_UNORDERED_MAP_NEEDED
template <bool any, class K, class V, class... Others>
struct range_traits<std::unordered_map<K, V, Others...>, any, true> {
  using reference    = std::pair<const K, V>&;
  using value_type   = std::pair<const K, V>;
  using iterator_tag = std::forward_iterator_tag;

  constexpr static bool           defined = is_defined_v<K> && is_defined_v<V>;
  constexpr static storage_type_t storage = storage_type_t::discontinuous;
  constexpr static range_type_t   range   = range_type_t::map_like;
};
template <bool any, class K, class V, class... Others>
struct range_traits<std::unordered_multimap<K, V, Others...>, any, true> {
  using reference    = std::pair<const K, V>&;
  using value_type   = std::pair<const K, V>;
  using iterator_tag = std::forward_iterator_tag;

  constexpr static bool           defined = is_defined_v<K> && is_defined_v<V>;
  constexpr static storage_type_t storage = storage_type_t::discontinuous;
  constexpr static range_type_t   range   = range_type_t::map_like;
};
#endif

#ifdef BXLX_GRAPH_RANGE_TRAITS_RANGES_NEEDED
#  undef BXLX_GRAPH_RANGE_TRAITS_RANGES_NEEDED
// probably it is not necessary this whole if.
/*
template <class M, bool any, bool any2>
      struct range_traits < M,
      any, any2, std::enable_if_t<std::is_base_of_v<std::ranges::view_interface<M>, M>>;
*/
#endif


template <class T>
using std_begin_t = decltype(std::begin(std::declval<T&>()));
template <class T>
using std_end_t = decltype(std::end(std::declval<T&>()));
template <class T>
using std_data_t = decltype(std::data(std::declval<T&>()));

template <class, class = void>
constexpr inline bool has_std_data_v = false;
template <class T>
constexpr inline bool has_std_data_v<T, std::void_t<std_data_t<T>>> = true;


template <class, class = void>
constexpr inline bool has_std_iterator_traits_v = false;
template <class It>
constexpr inline bool has_std_iterator_traits_v<It, std::void_t<typename std::iterator_traits<It>::value_type>> = true;


template <class It, class Sentinel, bool = has_std_iterator_traits_v<It>&& std::is_same_v<It, Sentinel>, class = void>
constexpr inline bool is_iterator_pair_v = false;
template <class It, class Sentinel>
constexpr inline bool is_iterator_pair_v<It, Sentinel, true> = true;
template <class It, class Sentinel>
constexpr inline bool is_iterator_pair_v<
      It,
      Sentinel,
      false,
      std::enable_if_t<std::is_convertible_v<decltype(std::declval<It>() != std::declval<Sentinel>()), bool> &&
                       class_member_traits::has_star_op_v<It> &&
                       std::is_same_v<class_member_traits::get_increment_op_result_t<It&>, It&>>> = true;

template <class T, bool = std::is_class_v<T>, class = void>
[[maybe_unused]] constexpr inline bool has_begin_end_iterators_v = false;
template <class T>
[[maybe_unused]] constexpr inline bool
      has_begin_end_iterators_v<T, true, std::enable_if_t<is_iterator_pair_v<std_begin_t<T>, std_end_t<T>>>> = true;

template <class It, class = void>
struct iterator_traits_impl;

template <class It>
struct iterator_traits_impl<It, std::enable_if_t<has_std_iterator_traits_v<It>>> {
  using reference                 = typename std::iterator_traits<It>::reference;
  using category [[maybe_unused]] = typename std::iterator_traits<It>::iterator_category;
};


template <class It>
struct iterator_traits_impl<It,
                            std::enable_if_t<!has_std_iterator_traits_v<It> && class_member_traits::has_star_op_v<It> &&
                                             class_member_traits::has_increment_op_v<It>>> {
  using reference                 = class_member_traits::get_star_op_result_t<It>;
  using category [[maybe_unused]] = std::conditional_t<has_subscript_operator_v<It>,
                                                       std::random_access_iterator_tag,
                                                       std::conditional_t<class_member_traits::has_decrement_op_v<It>,
                                                                          std::bidirectional_iterator_tag,
                                                                          std::forward_iterator_tag>>;
};


namespace associative_traits {
  template <class T, class ValueType, bool =
                                            class_member_traits::has_key_type_type_v<T> &&
                                                  is_tuple_v<ValueType>, class = void>
  constexpr static inline bool is_map_v = false;
  template <class T, class ValueType>
  constexpr static inline bool is_map_v<T, ValueType, true,
                                        std::enable_if_t<std::tuple_size_v<ValueType> == 2>> =
        std::is_same_v<class_member_traits::get_key_type_member_t<T>, std::remove_cv_t<std::tuple_element_t<0, ValueType>>>;

  template <class T, class ValueType, bool = class_member_traits::has_key_type_type_v<T>>
  constexpr static inline bool is_set_v = false;
  template <class T, class ValueType>
  constexpr static inline bool is_set_v<T, ValueType, true> =
        std::is_same_v<class_member_traits::get_key_type_member_t<T>, std::remove_cv_t<ValueType>>;
} // namespace associative_traits

template <class T, bool all_defined, class = void>
struct range_traits_impl {};

template <class T>
struct range_traits_impl<T, true, std::enable_if_t<has_begin_end_iterators_v<T>>> {
  using it_traits                   = iterator_traits_impl<std_begin_t<T>>;
  using reference [[maybe_unused]]  = typename it_traits::reference;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  using iterator_tag                = typename it_traits::category;

  constexpr static bool           defined = true;
  constexpr static storage_type_t storage = has_std_data_v<T> ? storage_type_t::continuous
                                                              : storage_type_t::discontinuous;

  constexpr static range_type_t range =
        class_member_traits::has_length_v<T> ? range_type_t::string_like
        : class_member_traits::has_push_front_v<std::remove_const_t<T>, std::add_rvalue_reference_t<value_type>> &&
                    class_member_traits::has_push_back_v<std::remove_const_t<T>,
                                                         std::add_rvalue_reference_t<value_type>>
              ? range_type_t::queue_like
        : associative_traits::is_map_v<T, value_type> ? range_type_t::map_like
        : associative_traits::is_set_v<T, value_type> ? range_type_t::set_like
                                                      : range_type_t::sequence;
};


template <class T, bool r, bool = !is_optional_v<T, r>>
struct range_traits_impl_helper {};

template <class T>
struct range_traits_impl_helper<T, true, true> : range_traits_impl<T, true> {};

template <class T>
struct range_traits_impl_helper<T, false, true> : range_traits_impl<T, is_defined_v<T>> {};


template <class M, bool any>
struct range_traits<M, any, true> : range_traits_impl_helper<M, any> {};

} // namespace bxlx::graph::type_classification::detail


#endif //BXLX_GRAPH_RANGE_TRAITS_HPP
