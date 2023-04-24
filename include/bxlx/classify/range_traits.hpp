//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_RANGE_TRAITS_HPP
#define BXLX_GRAPH_RANGE_TRAITS_HPP

#include "type_traits.hpp"

#if defined(_MSC_VER)
#include <array>
#endif

namespace bxlx::graph::type_traits::detail {

enum class range_type_t {
  sequence,
  string_like, /* has .length() */
  queue_like,  /* has .push_front() and .push_back() */
  map_like,    /* has key_type, equal to value_type first component */
  set_like     /* has key_type, equal to value_type */
};

template <class T,
          class U = std::remove_volatile_t<T>,
          class   = std::enable_if_t<std::is_same_v<T, U>>,
          class   = typename known_range<T>::value_type>
using enable_if_is_known_range = T;

template <class T>
struct known_range<volatile enable_if_is_known_range<T>> : known_range<T> {};

template <class T>
struct known_range<const volatile enable_if_is_known_range<const T>> : known_range<const T> {};

template <class T>
struct known_range<const enable_if_is_known_range<T>> : known_range<T> {
  using value_type     = std::add_const_t<typename known_range<T>::value_type>;
  using orig_reference = typename known_range<T>::reference;
  using reference      = std::conditional_t<std::is_lvalue_reference_v<orig_reference>,
                                       std::add_lvalue_reference_t<value_type>,
                                       std::conditional_t<std::is_rvalue_reference_v<orig_reference>,
                                                          std::add_rvalue_reference_t<orig_reference>,
                                                          orig_reference>>;
};

#if defined(_MSC_VER)
template <class M, std::size_t S>
struct known_range<std::array<M, S>> {
  using reference    = M&;
  using value_type   = M;
  using iterator_tag = std::random_access_iterator_tag;

  constexpr static bool         defined    = is_defined_v<M>;
  constexpr static bool         continuous = true;
  constexpr static range_type_t range      = range_type_t::sequence;
};
#endif

template <class T, bool any>
struct range_traits<T, any, std::enable_if_t<is_known_range_v<T>>> : known_range<T> {};

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
template <class T>
constexpr inline bool has_std_data_v<T, std::enable_if_t<class_member_traits::has_container_type_type_v<T>>> =
      has_std_data_v<class_member_traits::get_container_type_member_t<T>>;


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
  template <class T, class Val>
  constexpr static inline bool has_value_equal_range_function_v = class_member_traits::
        has_equal_range_res_v<const T, std::pair<std_begin_t<const T>, std_begin_t<const T>>, Val const&>;


  template <class Impl, class Val, bool = class_member_traits::has_key_type_type_v<Impl>, class = void>
  struct is_map_impl {
    template <class T>
    constexpr static inline bool has_map_equal_range_function_v =
          class_member_traits::has_equal_range_res_v<const T,
                                                     std::pair<std_begin_t<const T>, std_begin_t<const T>>,
                                                     std::tuple_element_t<0, Val> const&>;

    template <class T>
    constexpr static inline bool has_map_at_function_v =
          class_member_traits::has_at_v<const T, std::tuple_element_t<0, Val> const&>;

    [[maybe_unused]] constexpr static inline bool value =
          has_map_equal_range_function_v<Impl> &&
          (!has_value_equal_range_function_v<Impl, Val> || has_map_at_function_v<Impl>);
  };
  template <class Impl, class Val>
  struct is_map_impl<Impl, Val, true> {
    [[maybe_unused]] constexpr static inline bool value =
          std::is_same_v<class_member_traits::get_key_type_member_t<Impl>,
                         std::remove_cv_t<std::tuple_element_t<0, Val>>>;
  };

  template <class T, class ValueType, bool = is_tuple_v<ValueType>, class = void>
  constexpr static inline bool is_map_v = false;
  template <class T, class ValueType>
  constexpr static inline bool is_map_v<T, ValueType, true, std::enable_if_t<std::tuple_size_v<ValueType> == 2>> =
        is_map_impl<T, ValueType>::value;

  template <class T, class ValueType, bool = class_member_traits::has_key_type_type_v<T>>
  constexpr static inline bool is_set_v = !is_map_v<T, ValueType> && has_value_equal_range_function_v<T, ValueType>;
  template <class T, class ValueType>
  constexpr static inline bool is_set_v<T, ValueType, true> =
        std::is_same_v<class_member_traits::get_key_type_member_t<T>, std::remove_cv_t<ValueType>>;
} // namespace associative_traits

struct defined_range_key {
  constexpr bool operator<(const defined_range_key&) const { return false; }
  constexpr bool operator==(const defined_range_key&) const { return false; }
};

struct defined_range_value {
  constexpr bool operator<(const defined_range_value&) const { return false; }
  constexpr bool operator==(const defined_range_value&) const { return false; }
};

template <class T, bool possible_range, bool all_defined, class = void>
struct range_traits_impl {};

template <class T>
struct range_traits_impl<T, true, true, std::enable_if_t<has_begin_end_iterators_v<T>>> {
  using it_traits                   = iterator_traits_impl<std_begin_t<T>>;
  using reference [[maybe_unused]]  = typename it_traits::reference;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  using iterator_tag                = typename it_traits::category;

  constexpr static bool defined    = true;
  constexpr static bool continuous = has_std_data_v<T>;

  constexpr static range_type_t range =
        class_member_traits::has_length_v<T> ? range_type_t::string_like
        : class_member_traits::has_push_front_v<std::remove_const_t<T>,
                                                std::add_rvalue_reference_t<std::remove_const_t<value_type>>> &&
                    class_member_traits::has_push_back_v<std::remove_const_t<T>,
                                                         std::add_rvalue_reference_t<std::remove_const_t<value_type>>>
              ? range_type_t::queue_like
        : associative_traits::is_map_v<T, value_type> ? range_type_t::map_like
        : associative_traits::is_set_v<T, value_type> ? range_type_t::set_like
                                                      : range_type_t::sequence;
};

template <class Type, class From, class To>
struct replace_all_type_recursively {
  using type = Type;
};

template <class Type, class From, class To>
struct replace_all_type_recursively<Type&, From, To> {
  using type = typename replace_all_type_recursively<Type, From, To>::type&;
};

template <class Type, class From, class To>
struct replace_all_type_recursively<Type&&, From, To> {
  using type = typename replace_all_type_recursively<Type, From, To>::type&&;
};

template <class Type, class From, class To>
struct replace_all_type_recursively<const Type, From, To> {
  using type = const typename replace_all_type_recursively<Type, From, To>::type;
};

template <class Type, class From, class To>
struct replace_all_type_recursively<volatile Type, From, To> {
  using type = volatile typename replace_all_type_recursively<Type, From, To>::type;
};

template <class From, class To>
struct replace_all_type_recursively<From, From, To> {
  using type = To;
};

template <template <class...> class Typeof, class From, class To, class... Types>
struct replace_all_type_recursively<Typeof<Types...>, From, To> {
  using type = Typeof<typename replace_all_type_recursively<Types, From, To>::type...>;
};

template <template <class, auto> class Typeof, class From, class To, class Type, auto Val>
struct replace_all_type_recursively<Typeof<Type, Val>, From, To> {
  using type = Typeof<typename replace_all_type_recursively<Type, From, To>::type, Val>;
};

template <template <class, class, auto> class Typeof, class From, class To, class Type, class Type2, auto Val>
struct replace_all_type_recursively<Typeof<Type, Type2, Val>, From, To> {
  using type = Typeof<typename replace_all_type_recursively<Type, From, To>::type,
                      typename replace_all_type_recursively<Type2, From, To>::type,
                      Val>;
};
template <template <class, class, class, auto> class Typeof,
          class From,
          class To,
          class Type,
          class Type2,
          class Type3,
          auto Val>
struct replace_all_type_recursively<Typeof<Type, Type2, Type3, Val>, From, To> {
  using type = Typeof<typename replace_all_type_recursively<Type, From, To>::type,
                      typename replace_all_type_recursively<Type2, From, To>::type,
                      typename replace_all_type_recursively<Type3, From, To>::type,
                      Val>;
};


template <template <class, class...> class Range, class O, class... Other>
struct range_traits_impl<Range<O, Other...>,
                         true,
                         false,
                         std::enable_if_t<!is_defined_v<O> && one_required_templated_class<Range<O, Other...>>::value &&
                                          is_range_v<Range<defined_range_value>>>> {
  using reference [[maybe_unused]] =
        typename replace_all_type_recursively<range_reference_t<Range<defined_range_value>>, defined_range_value, O>::
              type;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  using iterator_tag                = range_iterator_tag_t<Range<defined_range_value>>;

  constexpr static bool defined    = true;
  constexpr static bool continuous = range_is_continuous_v<Range<defined_range_value>>;

  constexpr static range_type_t range = range_type_v<Range<defined_range_value>>;
};

template <template <class, class...> class Range, class O, class... Other>
struct range_traits_impl<const Range<O, Other...>,
                         true,
                         false,
                         std::enable_if_t<!is_defined_v<O> && one_required_templated_class<const Range<O, Other...>>::value &&
                                          is_range_v<const Range<defined_range_value>>>> {
  using reference [[maybe_unused]] =
        typename replace_all_type_recursively<range_reference_t<const Range<defined_range_value>>, defined_range_value, O>::
              type;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  using iterator_tag                = range_iterator_tag_t<const Range<defined_range_value>>;

  constexpr static bool defined    = true;
  constexpr static bool continuous = range_is_continuous_v<const Range<defined_range_value>>;

  constexpr static range_type_t range = range_type_v<const Range<defined_range_value>>;
};

template <template <class, auto, class...> class Range, class O, auto S, class... Other>
struct range_traits_impl<
      Range<O, S, Other...>,
      true,
      false,
      std::enable_if_t<!is_defined_v<O> && array_like_required_template_class<Range<O, S, Other...>>::value &&
                       is_range_v<Range<defined_range_value, S>>>> {
  using reference
        [[maybe_unused]] = typename replace_all_type_recursively<range_reference_t<Range<defined_range_value, S>>,
                                                                 defined_range_value,
                                                                 O>::type;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  using iterator_tag                = range_iterator_tag_t<Range<defined_range_value, S>>;

  constexpr static bool defined    = true;
  constexpr static bool continuous = range_is_continuous_v<Range<defined_range_value, S>>;

  constexpr static range_type_t range = range_type_v<Range<defined_range_value, S>>;
};


template <template <class, auto, class...> class Range, class O, auto S, class... Other>
struct range_traits_impl<
      const Range<O, S, Other...>,
      true,
      false,
      std::enable_if_t<!is_defined_v<O> && array_like_required_template_class<const Range<O, S, Other...>>::value &&
                       is_range_v<const Range<defined_range_value, S>>>> {
  using reference
        [[maybe_unused]] = typename replace_all_type_recursively<range_reference_t<const Range<defined_range_value, S>>,
                                                                 defined_range_value,
                                                                 O>::type;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  using iterator_tag                = range_iterator_tag_t<const Range<defined_range_value, S>>;

  constexpr static bool defined    = true;
  constexpr static bool continuous = range_is_continuous_v<const Range<defined_range_value, S>>;

  constexpr static range_type_t range = range_type_v<const Range<defined_range_value, S>>;
};

template <bool, class...>
struct undef_if_false;

template <class... Classes>
struct undef_if_false<true, Classes...> {
  constexpr static bool value = true;
};

template <class R, bool = two_required_templated_class<R>::value>
struct replaced_is_range {
  using type = R;
};
template <template <class, class, class...> class Range, class K, class M, class... Other>
struct replaced_is_range<Range<K, M, Other...>, true> {
  using type = Range<std::conditional_t<is_defined_v<K>, K, defined_range_key>,
                     std::conditional_t<is_defined_v<M>, M, defined_range_value>>;

  constexpr static bool value = is_range_v<type>;
};
template <template <class, class, class...> class Range, class K, class M, class... Other>
struct replaced_is_range<const Range<K, M, Other...>, true> {
  using type = const Range<std::conditional_t<is_defined_v<K>, K, defined_range_key>,
                     std::conditional_t<is_defined_v<M>, M, defined_range_value>>;

  constexpr static bool value = is_range_v<type>;
};


template <template <class, class, class...> class Range, class K, class M, class... Other>
struct range_traits_impl<
      Range<K, M, Other...>,
      true,
      false,
      std::enable_if_t<(!is_defined_v<K> || !is_defined_v<M>)&&replaced_is_range<Range<K, M, Other...>>::value>> {
  using the_range = Range<std::conditional_t<is_defined_v<K>, K, defined_range_key>,
                          std::conditional_t<is_defined_v<M>, M, defined_range_value>>;
  using rref      = range_reference_t<the_range>;

  using reference [[maybe_unused]] =
        typename replace_all_type_recursively<typename replace_all_type_recursively<rref, defined_range_key, K>::type,
                                              defined_range_value,
                                              M>::type;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  using iterator_tag                = range_iterator_tag_t<the_range>;

  constexpr static bool defined    = true;
  constexpr static bool continuous = range_is_continuous_v<the_range>;

  constexpr static range_type_t range = range_type_v<the_range>;
};

template <template <class, class, class...> class Range, class K, class M, class... Other>
struct range_traits_impl<
      const Range<K, M, Other...>,
      true,
      false,
      std::enable_if_t<(!is_defined_v<K> || !is_defined_v<M>)&&replaced_is_range<const Range<K, M, Other...>>::value>> {
  using the_range = const Range<std::conditional_t<is_defined_v<K>, K, defined_range_key>,
                          std::conditional_t<is_defined_v<M>, M, defined_range_value>>;
  using rref      = range_reference_t<the_range>;

  using reference [[maybe_unused]] =
        typename replace_all_type_recursively<typename replace_all_type_recursively<rref, defined_range_key, K>::type,
                                              defined_range_value,
                                              M>::type;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  using iterator_tag                = range_iterator_tag_t<the_range>;

  constexpr static bool defined    = true;
  constexpr static bool continuous = range_is_continuous_v<the_range>;

  constexpr static range_type_t range = range_type_v<the_range>;
};

template <class M>
struct range_traits<M, true, std::enable_if_t<!is_known_range_v<M> && required_template_arguments_defined_v<M>>>
      : range_traits_impl<M, !is_known_optional_v<M>, true> {};

template <class M>
struct range_traits<M, true, std::enable_if_t<!is_known_range_v<M> && !required_template_arguments_defined_v<M>>>
      : range_traits_impl<M, !is_optional_v<M>, false> {};

template <class T>
struct is_string<T, false> : std::false_type {};

template <class T>
struct is_string<T, true> : std::bool_constant<range_type_v<T> == range_type_t::string_like> {};

} // namespace bxlx::graph::type_traits::detail

namespace std {
template <>
struct hash<bxlx::graph::type_traits::detail::defined_range_key> {
  constexpr std::size_t operator()(const bxlx::graph::type_traits::detail::defined_range_key&) const { return {}; }
};
template <>
struct hash<bxlx::graph::type_traits::detail::defined_range_value> {
  constexpr std::size_t operator()(const bxlx::graph::type_traits::detail::defined_range_value&) const { return {}; }
};
} // namespace std

#endif //BXLX_GRAPH_RANGE_TRAITS_HPP
