//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TYPE_CLASSIFICATION_HPP
#define BXLX_GRAPH_TYPE_CLASSIFICATION_HPP

#include <optional>
#include <tuple>
#include <type_traits>

namespace bxlx::graph::type_classification {

template <class, class = void>
constexpr inline bool is_tuple_v = false;
template <class T>
constexpr inline bool is_tuple_v<T, std::void_t<decltype(std::tuple_size<T>::value)>> = std::tuple_size_v<T>;

namespace detail {
  template <class T>
  struct template_inspect {
    using types = std::tuple<>;
  };

  /*
  template <template <class> class TT, class T>
  struct template_inspect<TT<T>> {
    using types = std::tuple<T>;
  };

  template <template <class, class> class TT, class T, class U>
  struct template_inspect<TT<T, U>> {
    using types = std::tuple<T, U>;
  };
   */

  template <template <class...> class TT, class... Ts>
  struct template_inspect<TT<Ts...>> {
    using types = std::tuple<Ts...>;
  };

  template <template <class, auto> class TT, class T, auto U>
  struct template_inspect<TT<T, U>> {
    using types = std::tuple<T, std::integral_constant<decltype(U), U>>;
  };

  template <class T, std::size_t = sizeof(T)>
  [[maybe_unused]] constexpr std::true_type defined_type(int);
  template <class>
  constexpr std::false_type defined_type(...);

  template <class T, class = void>
  struct is_defined : decltype(defined_type<T>(0)) {};

  template <class T>
  constexpr inline bool is_defined_v = is_defined<T>::value;

  template <class T,
            class TL = typename template_inspect<T>::types,
            class    = std::make_index_sequence<std::tuple_size_v<TL>>>
  struct all_template_defined : std::true_type {
    static_assert(std::tuple_size_v<TL> == 0);
  };

  template <class T, class TL, template <class, std::size_t...> class TT, std::size_t... ix>
  struct all_template_defined<T, TL, TT<std::size_t, ix...>>
        : std::conjunction<is_defined<std::tuple_element_t<ix, TL>>...> {};

  template <class T>
  struct is_defined<T, std::enable_if_t<is_tuple_v<T>>> : all_template_defined<T, T> {};

  template <class T>
  struct is_defined<T, std::enable_if_t<std::is_array_v<T>>> : is_defined<std::remove_extent_t<T>> {};

  template <class T,
            bool  = !is_tuple_v<T> && !std::is_array_v<T> && !std::is_void_v<std::remove_pointer_t<T>>>
  struct optional_traits;

  template <class T,
            class U = std::remove_cv_t<T>,
            class   = std::enable_if_t<std::is_same_v<T, U>>,
            class   = typename optional_traits<T>::value_type>
  using enable_if_is_optional = T;

  template <class T, bool any>
  struct optional_traits<volatile enable_if_is_optional<T>, any> : optional_traits<T> {};

  template <class T, bool any>
  struct optional_traits<const volatile enable_if_is_optional<const T>, any> : optional_traits<const T> {};


  template <class V>
  struct optional_traits<std::optional<V>, true> {
    using value_type = V;
    using reference  = V&;
  };

  template <class V>
  struct optional_traits<const std::optional<V>, true> {
    using value_type = V const;
    using reference  = V const&;
  };

  template <class T, bool all_defined = all_template_defined<T>::value, class = void>
  struct optional_traits_impl {};

  template <class T>
  struct optional_traits_impl<
        T,
        true,
        std::void_t<std::enable_if_t<std::is_convertible_v<T, bool>>, decltype(*std::declval<T>())>> {
    using reference [[maybe_unused]]  = decltype(*std::declval<T&>());
    using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  };

  template <class T>
  struct optional_traits<T, true>
        : optional_traits_impl<T> {};

  template <class T>
  struct is_defined<T, std::void_t<typename detail::optional_traits<T>::value_type>>
        : is_defined<typename detail::optional_traits<T>::value_type> {};


  template <class From, class To, bool IsConvertible = std::is_convertible_v<From, To>>
  struct is_nothrow_convertible_impl : std::false_type {};
  template <class From, class To>
  struct is_nothrow_convertible_impl<From, To, true> {
    static void test(To) noexcept {}

    [[maybe_unused]] constexpr static inline bool value = noexcept(test(std::declval<From>()));
  };

  template <class From, class To>
  constexpr inline auto is_nothrow_convertible_v = is_nothrow_convertible_impl<From, To>::value;


  template <class Helper, class Type, class... Args>
  struct member_function_invoke_result {
    using T = std::remove_cv_t<std::remove_reference_t<Type>>;

    constexpr static bool is_const  = std::is_const_v<std::remove_reference_t<Type>>;
    constexpr static bool is_lv_ref = std::is_lvalue_reference_v<Type>;
    constexpr static bool is_rv_ref = std::is_rvalue_reference_v<Type>;
    template <class Res = Helper>
    auto operator()(Res (T::*)(Args...)) -> std::enable_if_t<!is_const, Res>;
    template <class Res = Helper>
    auto operator()(Res (T::*)(Args...) const) const -> Res;
    template <class Res = Helper>
    auto operator()(Res (T::*)(Args...) &) -> std::enable_if_t<!is_rv_ref, Res>;
    template <class Res = Helper>
    auto operator()(Res (T::*)(Args...) const&) const -> Res;
    template <class Res = Helper>
    auto operator()(Res (T::*)(Args...) &&) const volatile -> std::enable_if_t<!is_lv_ref, Res>;
    // volatile and const&& overloads are not handled by design.
  };

  template <class Helper, class T, class... Args>
  inline member_function_invoke_result<Helper, T, Args...> member_function_invoke_result_v{};

  template <class T, class, bool = std::is_class_v<std::remove_reference_t<T>>&& is_defined_v<T>, class = void>
  constexpr static auto has_conversion_operator_v = false;
  template <class T, class U>
  constexpr static auto has_conversion_operator_v<
        T,
        U,
        true,
        std::void_t<decltype(member_function_invoke_result_v<U, T>(&std::remove_reference_t<T>::operator U))>> = true;

  template <class T, class U>
  constexpr static inline bool has_any_conversion_operator =
        has_conversion_operator_v<T, U> || has_conversion_operator_v<T, U&> || has_conversion_operator_v<T, const U&> ||
        has_conversion_operator_v<T, U&&> || has_conversion_operator_v<T, const U&&>;
} // namespace detail

template <class T>
using optional_value_t = typename detail::optional_traits<T>::value_type;
template <class T>
using optional_reference_t = typename detail::optional_traits<T>::reference;

template <class, class = void>
constexpr inline bool is_optional_v = false;
template <class T>
constexpr inline bool is_optional_v<T, std::void_t<optional_value_t<T>>> = true;


template <class T, bool = detail::is_defined_v<T>>
constexpr inline bool is_bool_ref_v = false;
template <class T>
constexpr inline bool is_bool_ref_v<T, true> =        // type must be defined
      std::is_class_v<T> &&                           // bool ref can be only classes, whose
      detail::is_nothrow_convertible_v<T, bool> &&    // can convert to bool
      detail::has_any_conversion_operator<T, bool> && // with bool conversion operator
      !std::is_constructible_v<T, bool&>;             // cannot construct from bool&

template <class T>
constexpr inline bool is_bool_v = std::is_same_v<std::remove_cv_t<T>, bool> || is_bool_ref_v<T>;


template <class T, bool = detail::is_defined_v<T>>
constexpr inline bool is_size_t_v = false;
template <class T>
constexpr inline bool is_size_t_v<T, true> =              // type must be defined
      std::is_class_v<T> &&                               // size_t wrapper can be only classes, whose
      detail::is_nothrow_convertible_v<T, std::size_t> && // can convert to size_t
      !detail::has_any_conversion_operator<T, bool> &&    // but not with operator bool. accept char-s
      detail::is_nothrow_convertible_v<std::size_t, T>;   // can convert from size_t


template <class CVT, class T = std::remove_cv_t<CVT>>
constexpr inline bool is_char_v =
      std::is_same_v<T, char> || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t> ||
      std::is_same_v<T, wchar_t> || std::is_same_v<T, decltype(u8'\0')>;
// C++17 -> u8'\0' is same type as char, but C++20 it is char8_t, which is different from char


template <class T>
constexpr inline bool is_index_v =
      !std::is_same_v<bool, std::remove_cv_t<T>> && !is_char_v<T> && (std::is_integral_v<T> || is_size_t_v<T>);

} // namespace bxlx::graph::type_classification

#endif //BXLX_GRAPH_TYPE_CLASSIFICATION_HPP
