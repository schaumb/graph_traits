//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TYPE_TRAITS_HPP
#define BXLX_GRAPH_TYPE_TRAITS_HPP

#include <iterator>
#include <tuple>
#include <type_traits>

#include "member_traits.hpp"

namespace bxlx::graph::type_traits {

template <class, class = void>
constexpr inline bool is_tuple_v = false;
template <class T>
constexpr inline bool is_tuple_v<T, std::void_t<decltype(std::tuple_size<T>::value)>> = std::tuple_size_v<T>;

namespace detail {
  template <class T>
  struct template_inspect {
    using types = std::tuple<>;
  };

  template <template <class...> class TT, class... Ts>
  struct template_inspect<TT<Ts...>> {
    using types = std::tuple<Ts...>;
  };

  template <template <class, auto> class TT, class T, auto U>
  struct template_inspect<TT<T, U>> {
    using types = std::tuple<T, std::integral_constant<decltype(U), U>>;
  };

  template <template <class, auto, class...> class TT, class T, auto U, class... Oth>
  struct template_inspect<TT<T, U, Oth...>> {
    using types = std::tuple<T, std::integral_constant<decltype(U), U>, Oth...>;
  };

  template <class From, class To>
  using copy_reference_t =
        std::conditional_t<std::is_lvalue_reference_v<From>,
                           std::add_lvalue_reference_t<To>,
                           std::conditional_t<std::is_rvalue_reference_v<From>, std::add_rvalue_reference_t<To>, To>>;

  template <class From, class To>
  struct copy_cvref {
    using from_without_ref = std::remove_reference_t<From>;
    using copied_const     = std::conditional_t<std::is_const_v<from_without_ref>, std::add_const_t<To>, To>;
    using copied_cv =
          std::conditional_t<std::is_volatile_v<from_without_ref>, std::add_volatile_t<copied_const>, copied_const>;
    using type = copy_reference_t<From, copied_cv>;
  };

  template <class From, class To>
  using copy_cvref_t = typename copy_cvref<From, To>::type;

  template <class T, std::size_t = sizeof(T)>
  [[maybe_unused]] constexpr std::true_type defined_type(int);
  template <class>
  constexpr std::false_type defined_type(...);

  template <class T>
  using is_sizeof_gettable = decltype(defined_type<T>(0));

  template <class T, class = void>
  struct is_defined : is_sizeof_gettable<T> {};

  template <>
  struct is_defined<std::true_type> : std::true_type {};
  template <>
  struct is_defined<std::false_type> : std::true_type {};

  template <class T>
  struct is_defined<T, std::enable_if_t<std::is_function_v<T>>> : std::true_type {};

  template <class T>
  constexpr inline bool is_defined_v = is_defined<T>::value;

  template <class T,
            class TL = typename template_inspect<std::remove_cv_t<T>>::types,
            class    = std::make_index_sequence<std::tuple_size_v<TL>>>
  struct all_template_defined : std::true_type {
    static_assert(std::tuple_size_v<TL> == 0);
  };

  template <class T, class TL, std::size_t... ix>
  struct all_template_defined<T, TL, std::index_sequence<ix...>>
        : std::conjunction<is_defined<std::tuple_element_t<ix, TL>>...> {};


  template <class T, bool = !is_tuple_v<T>, class = void>
  struct one_required_templated_class : std::false_type {};

  template <template <class, class...> class T, class U, class... Vs>
  struct one_required_templated_class<T<U, Vs...>, true, std::void_t<T<U>>> : std::true_type {
    constexpr static bool is_defined = is_defined_v<U>;
  };

  template <template <class, class...> class T, class U, class... Vs>
  struct one_required_templated_class<const T<U, Vs...>, true, std::void_t<T<U>>> : std::true_type {
    constexpr static bool is_defined = is_defined_v<U>;
  };

  template <class T, bool = !is_tuple_v<T>, class = void>
  struct two_required_templated_class : std::false_type {};

  template <template <class, class, class...> class T, class U, class V, class... Vs>
  struct two_required_templated_class<
        T<U, V, Vs...>, true,
        std::void_t<std::enable_if_t<!one_required_templated_class<T<U, V, Vs...>>::value>, T<U, V>>> : std::true_type {
    constexpr static bool is_defined = is_defined_v<U> && is_defined_v<V>;
  };

  template <template <class, class, class...> class T, class U, class V, class... Vs>
  struct two_required_templated_class<
        const T<U, V, Vs...>, true,
        std::void_t<std::enable_if_t<!one_required_templated_class<T<U, V, Vs...>>::value>, T<U, V>>> : std::true_type {
    constexpr static bool is_defined = is_defined_v<U> && is_defined_v<V>;
  };

  template <class T, class = void>
  struct array_like_required_template_class : std::false_type {};

  template <template <class, auto, class...> class T, class U, auto V, class... Vs>
  struct array_like_required_template_class<
        T<U, V, Vs...>,
        std::void_t<std::enable_if_t<!one_required_templated_class<T<U, V, Vs...>>::value>, T<U, V>>> : std::true_type {
    constexpr static bool is_defined = is_defined_v<U>;
  };

  template <template <class, auto, class...> class T, class U, auto V, class... Vs>
  struct array_like_required_template_class<
        const T<U, V, Vs...>,
        std::void_t<std::enable_if_t<!one_required_templated_class<T<U, V, Vs...>>::value>, T<U, V>>> : std::true_type {
    constexpr static bool is_defined = is_defined_v<U>;
  };


  template <class T, class = void>
  constexpr bool required_template_arguments_defined_v = all_template_defined<T>::value;

  template <class T>
  constexpr bool required_template_arguments_defined_v<T, std::enable_if_t<one_required_templated_class<T>::value>> =
        one_required_templated_class<T>::is_defined;

  template <class T>
  constexpr bool required_template_arguments_defined_v<T, std::enable_if_t<two_required_templated_class<T>::value>> =
        two_required_templated_class<T>::is_defined;

  template <class T>
  constexpr bool
        required_template_arguments_defined_v<T, std::enable_if_t<array_like_required_template_class<T>::value>> =
              array_like_required_template_class<T>::is_defined;

  template <class T>
  struct is_defined<T, std::enable_if_t<is_tuple_v<T>>> : all_template_defined<T, T> {};

  template <class T>
  struct is_defined<T, std::enable_if_t<std::is_array_v<T>>> : is_defined<std::remove_extent_t<T>> {};

  template <class T,
            bool = !is_tuple_v<T> && !std::is_array_v<T> && !std::is_void_v<std::remove_pointer_t<T>> &&
                   !std::is_function_v<T>,
            class = void>
  struct optional_traits;

  template <class, class = void>
  constexpr inline bool is_optional_v = false;
  template <class T>
  constexpr inline bool is_optional_v<T, std::void_t<typename optional_traits<T>::value_type>> = true;

  template <class T>
  struct is_defined<T, std::enable_if_t<is_optional_v<T>>> : is_defined<typename optional_traits<T>::value_type> {};


  template <class T, bool = true, class = void>
  struct range_traits;

  enum class range_type_t;

  template <class, class = void>
  constexpr inline bool is_range_v = false;
  template <class T>
  constexpr inline bool is_range_v<T, std::void_t<typename range_traits<T>::value_type>> = true;

  template <class T>
  struct is_defined<T, std::enable_if_t<is_range_v<T> && !is_tuple_v<T>>>
        : std::bool_constant<range_traits<T>::defined> {};

  template <class From, class To, bool IsConvertible = std::is_convertible_v<From, To>>
  struct is_nothrow_convertible_impl : std::false_type {};
  template <class From, class To>
  struct is_nothrow_convertible_impl<From, To, true> {
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4267)
#endif
    static void test(To) noexcept {}

    constexpr static inline bool value = noexcept(test(std::declval<From>()));

#ifdef _MSC_VER
#  pragma warning(pop)
#endif
  };

  template <class From, class To>
  [[maybe_unused]] constexpr inline auto is_nothrow_convertible_v = is_nothrow_convertible_impl<From, To>::value;

  template <class T, class, bool = std::is_class_v<std::remove_reference_t<T>>&& is_defined_v<T>, class = void>
  constexpr static auto has_conversion_operator_v = false;
  template <class T, class U>
  constexpr static auto
        has_conversion_operator_v<T, U, true, std::enable_if_t<class_member_traits::has_conversion_v<T, U>>> = true;

  template <class T, class U>
  [[maybe_unused]] constexpr static inline bool has_any_conversion_operator_v =
        has_conversion_operator_v<T, U> || has_conversion_operator_v<T, U&> || has_conversion_operator_v<T, const U&> ||
        has_conversion_operator_v<T, U&&> || has_conversion_operator_v<T, const U&&>;

  template <class T, bool = detail::is_defined_v<T> && !is_range_v<T> && !is_optional_v<T>>
  constexpr inline bool is_bool_ref_v = false;
  template <class T>
  constexpr inline bool is_bool_ref_v<T, true> =          // type must be defined
        std::is_class_v<T> &&                             // bool ref can be only classes, whose
        detail::is_nothrow_convertible_v<T, bool> &&      // can convert to bool
        detail::has_any_conversion_operator_v<T, bool> && // with bool conversion operator
        !std::is_constructible_v<T, bool&>;               // cannot construct from bool&

  template <class T, bool = detail::is_defined_v<T> && !is_range_v<T> && !is_optional_v<T>>
  constexpr inline bool is_size_t_v = false;
  template <class T>
  constexpr inline bool is_size_t_v<T, true> =              // type must be defined
        std::is_class_v<T> &&                               // size_t wrapper can be only classes, whose
        detail::is_nothrow_convertible_v<T, std::size_t> && // can convert to size_t
        !detail::has_any_conversion_operator_v<T, bool> &&  // but not with operator bool. accept char-s
        detail::is_nothrow_convertible_v<std::size_t, T>;   // can convert from size_t

  template <class T>
  using std_size_t = decltype(std::size(std::declval<T&>()));

  template <class T, bool = is_defined_v<T>, class = void>
  constexpr inline bool has_std_size_v = false;
  template <class T>
  constexpr inline bool has_std_size_v<T, true, std::void_t<std_size_t<T>>> = true;

  template <class T, class With = void, bool = has_std_size_v<T>, class = void>
  struct subscript_operator_traits {};
  template <class T>
  struct subscript_operator_traits<T, void, true> : subscript_operator_traits<T, std_size_t<T>> {};
  template <class T>
  struct subscript_operator_traits<T, void, false> : subscript_operator_traits<T, std::size_t, is_defined_v<T>> {};
  template <class T, class With, bool has_size>
  struct subscript_operator_traits<
        T,
        With,
        has_size,
        std::enable_if_t<!std::is_void_v<With> && class_member_traits::has_subscript_op_v<T, With>>> {
    using type [[maybe_unused]] = class_member_traits::get_subscript_op_result_t<T, With>;
  };

  template <class T, class With = void>
  using subscript_operator_return_t = typename subscript_operator_traits<T, With>::type;

  template <class T, class = void>
  constexpr inline bool has_subscript_operator_v = false;
  template <class T>
  constexpr inline bool has_subscript_operator_v<T, std::void_t<subscript_operator_return_t<T>>> = true;

  template <class T, bool = detail::has_std_size_v<T>, class = void>
  struct bitset_traits {};

  template <class T>
  struct bitset_traits<T, true, std::enable_if_t<has_subscript_operator_v<T>>> {
    using reference = subscript_operator_return_t<std::remove_const_t<T>>;
  };

  template <class CVT, class T = std::remove_cv_t<CVT>>
  constexpr inline bool is_char_v = std::is_same_v<T, char> || std::is_same_v<T, char16_t> ||
                                    std::is_same_v<T, char32_t> || std::is_same_v<T, wchar_t> ||
                                    std::is_same_v<T, decltype(u8'\0')>;
  // C++17 -> u8'\0' is same type as char, but C++20 it is char8_t, which is different from char

  template <class T, bool = is_range_v<T>>
  struct is_string;

  template <class T>
  constexpr inline bool is_string_v = is_string<T>::value;
} // namespace detail

template <class T>
using optional_value_t = typename detail::optional_traits<T>::value_type;
template <class T>
using optional_reference_t = typename detail::optional_traits<T>::reference;

template <class T>
constexpr inline bool is_optional_v = detail::is_optional_v<T>;

template <class T>
using range_value_t = typename detail::range_traits<T>::value_type;
template <class T>
using range_reference_t = typename detail::range_traits<T>::reference;
template <class T>
using range_iterator_tag_t = typename detail::range_traits<T>::iterator_tag;

using detail::range_type_t;

template <class T>
constexpr inline range_type_t range_type_v = detail::range_traits<T>::range;

template <class T>
constexpr inline bool range_is_continuous_v = detail::range_traits<T>::continuous;

template <class T>
constexpr inline bool is_range_v = detail::is_range_v<T> && !detail::is_string_v<T>;

template <class T>
[[maybe_unused]] constexpr inline bool is_bool_v =
      std::is_same_v<std::remove_cv_t<T>, bool> || detail::is_bool_ref_v<T>;

template <class T>
[[maybe_unused]] constexpr inline bool is_index_v =
      !std::is_same_v<bool, std::remove_cv_t<T>> && !detail::is_char_v<T> &&
      (std::is_integral_v<T> || detail::is_size_t_v<T> || std::is_enum_v<T>);

template <class T>
using bitset_reference_t = typename detail::bitset_traits<T>::reference;

template <class T, bool = std::is_class_v<T>&& detail::all_template_defined<T>::value, class = void>
constexpr inline bool is_bitset_v = false;
template <class T>
constexpr inline bool is_bitset_v<T, true, std::enable_if_t<detail::is_bool_ref_v<bitset_reference_t<T>>>> = true;

template <class T>
constexpr inline bool is_defined_v = detail::is_defined_v<T>;

} // namespace bxlx::graph::type_traits

#endif //BXLX_GRAPH_TYPE_TRAITS_HPP
