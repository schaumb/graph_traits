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


#ifdef _MSC_VER
#  include <deque>
#  define BXLX_GRAPH_RANGE_TRAITS_DEQUE_NEEDED 1
#  include <regex>
#  define BXLX_GRAPH_RANGE_TRAITS_REGEX_NEEDED 1
#elif (defined(__clang__) && __clang_major__ < 15) || (!defined(__clang__) && defined(__GNUC__) && __GNUC__ < 12)
#  include <unordered_set>
#  define BXLX_GRAPH_RANGE_TRAITS_USET_NEEDED 1
#  include <unordered_map>
#  define BXLX_GRAPH_RANGE_TRAITS_UMAP_NEEDED 1
#endif

#ifdef __cpp_lib_ranges
#  include <ranges>
#  define BXLX_GRAPH_RANGE_TRAITS_RANGES_NEEDED 1
#endif

namespace bxlx::graph::type_classification {
namespace detail {
  enum class range_type_t { sequence, set_like, map_like };

  template <class T, bool = !is_optional_v<T> && !is_tuple_v<T>, class = void>
  struct range_traits;

  template <class T,
            class U = std::remove_cv_t<T>,
            class   = std::enable_if_t<std::is_same_v<T, U>>,
            class   = typename range_traits<T>::value_type>
  using enable_if_is_range = T;

  template <class T, bool any>
  struct range_traits<volatile enable_if_is_range<T>, any> : range_traits<T> {};

  template <class T, bool any>
  struct range_traits<const volatile enable_if_is_range<const T>, any> : range_traits<const T> {};

  template <class T, bool any>
  struct range_traits<const enable_if_is_range<T>, any> : range_traits<T> {
    using value_type     = std::add_const_t<typename range_traits<T>::value_type>;
    using orig_reference = typename range_traits<T>::reference;
    using reference      = std::conditional_t<std::is_lvalue_reference_v<orig_reference>,
                                         std::add_lvalue_reference_t<value_type>,
                                         std::conditional_t<std::is_rvalue_reference_v<orig_reference>,
                                                            std::add_rvalue_reference_t<orig_reference>,
                                                            orig_reference>>;
  };

  template <bool any, class M, std::size_t S>
  struct range_traits<std::array<M, S>, any> {
    using reference    = M&;
    using value_type   = M;
    using iterator_tag = std::random_access_iterator_tag;

    constexpr static bool         contiguous    = true;
    constexpr static range_type_t associativity = range_type_t::sequence;
  };

  template <bool any, class CharT, class... Others>
  struct range_traits<std::basic_string<CharT, Others...>, any> {
    using reference    = CharT&;
    using value_type   = CharT;
    using iterator_tag = std::random_access_iterator_tag;

    constexpr static bool         contiguous    = true;
    constexpr static range_type_t associativity = range_type_t::sequence;
  };

  template <bool any, class CharT, class... Others>
  struct range_traits<std::basic_string_view<CharT, Others...>, any> {
    using reference    = CharT&;
    using value_type   = CharT;
    using iterator_tag = std::random_access_iterator_tag;

    constexpr static bool         contiguous    = true;
    constexpr static range_type_t associativity = range_type_t::sequence;
  };

#ifdef BXLX_GRAPH_RANGE_TRAITS_DEQUE_NEEDED
#undef BXLX_GRAPH_RANGE_TRAITS_DEQUE_NEEDED
  template <bool any, class M, class... Others>
  struct range_traits<std::deque<M, Others...>, any> {
    using reference    = M&;
    using value_type   = M;
    using iterator_tag = std::random_access_iterator_tag;

    constexpr static bool         contiguous    = false;
    constexpr static range_type_t associativity = range_type_t::sequence;
  };
#endif

#ifdef BXLX_GRAPH_RANGE_TRAITS_REGEX_NEEDED
#undef BXLX_GRAPH_RANGE_TRAITS_REGEX_NEEDED
  template <bool any, class M, class... Others>
  struct range_traits<std::match_results<M, Others...>, any> {
    using reference    = const std::sub_match<M>&;
    using value_type   = std::sub_match<M>;
    using iterator_tag = std::random_access_iterator_tag;

    constexpr static bool         contiguous    = false;
    constexpr static range_type_t associativity = range_type_t::sequence;
  };
#endif

#ifdef BXLX_GRAPH_RANGE_TRAITS_USET_NEEDED
#undef BXLX_GRAPH_RANGE_TRAITS_USET_NEEDED
  template <bool any, class M, class... Others>
  struct range_traits<std::unordered_set<M, Others...>, any> {
    using reference    = M&;
    using value_type   = M;
    using iterator_tag = std::forward_iterator_tag;

    constexpr static bool         contiguous    = false;
    constexpr static range_type_t associativity = range_type_t::set_like;
  };
  template <bool any, class M, class... Others>
  struct range_traits<std::unordered_multiset<M, Others...>, any> {
    using reference    = M&;
    using value_type   = M;
    using iterator_tag = std::forward_iterator_tag;

    constexpr static bool         contiguous    = false;
    constexpr static range_type_t associativity = range_type_t::set_like;
  };
#endif

#ifdef BXLX_GRAPH_RANGE_TRAITS_UMAP_NEEDED
#undef BXLX_GRAPH_RANGE_TRAITS_UMAP_NEEDED
  template <bool any, class K, class V, class... Others>
  struct range_traits<std::unordered_map<K, V, Others...>, any> {
    using reference    = std::pair<const K, V>&;
    using value_type   = std::pair<const K, V>;
    using iterator_tag = std::forward_iterator_tag;

    constexpr static bool         contiguous    = false;
    constexpr static range_type_t associativity = range_type_t::map_like;
  };
  template <bool any, class K, class V, class... Others>
  struct range_traits<std::unordered_multimap<K, V, Others...>, any> {
    using reference    = std::pair<const K, V>&;
    using value_type   = std::pair<const K, V>;
    using iterator_tag = std::forward_iterator_tag;

    constexpr static bool         contiguous    = false;
    constexpr static range_type_t associativity = range_type_t::map_like;
  };
#endif

#ifdef BXLX_GRAPH_RANGE_TRAITS_RANGES_NEEDED
#undef BXLX_GRAPH_RANGE_TRAITS_RANGES_NEEDED
// probably it is not necessary this whole if.
#define HAS_X(m) false
  template <class M, bool any>
  struct range_traits<M, any, std::enable_if_t<std::is_base_of_v<std::ranges::view_interface<M>, M>> {
    using IF = std::ranges::view_interface<M>;

    using reference = decltype(member_function_invoke_result_v<IF>(&IF::front));
    using value_type = std::remove_reference_t<reference>;
    using iterator_tag =
          HAS_X(&IF::operator[])
          ? std::random_access_iterator_tag
          : HAS_X(&IF::back)
                  ? std::bidirectional_iterator_tag
                  : std::forward_iterator_tag;

    constexpr static bool         contiguous    = HAS_X(&IF::data);
    constexpr static range_type_t associativity = range_type_t::sequence;
  };
#undef HAS_X
#endif

  template <class M>
  struct range_traits<M, true> {};

} // namespace detail
} // namespace bxlx::graph::type_classification


#endif //BXLX_GRAPH_RANGE_TRAITS_HPP
