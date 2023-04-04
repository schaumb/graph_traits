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

namespace detail {
  template <class T>
  struct template_inspect {};

  template <template <class> class TT, class T>
  struct template_inspect<TT<T>> {
    using types = std::tuple<T>;
  };

  template <template <class, class> class TT, class T, class U>
  struct template_inspect<TT<T, U>> {
    using types = std::tuple<T, U>;
  };

  template <template <class...> class TT, class... Ts>
  struct template_inspect<TT<Ts...>> {
    using types = std::tuple<Ts...>;
  };

  template <template <class, auto> class TT, class T, auto U>
  struct template_inspect<TT<T, U>> {
    using types = std::tuple<T, std::integral_constant<decltype(U), U>>;
  };
} // namespace detail

template <class CVT, class T = std::remove_cv_t<CVT>>
constexpr inline bool is_char_v =
      std::is_same_v<T, char> || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t> ||
      std::is_same_v<T, wchar_t> || std::is_same_v<T, decltype(u8'\0')>;
// C++17 -> u8'\0' is same type as char, but C++20 it is char8_t, which is different from char

template <class, class = void>
constexpr inline bool is_tuple_v = false;
template <class T>
constexpr inline bool is_tuple_v<T, std::void_t<decltype(std::tuple_size<T>::value)>> = std::tuple_size_v<T>;


namespace impl {
  template <class, class = void>
  struct optional_traits;

  template <class T,
            class U = std::remove_cv_t<T>,
            class   = std::enable_if_t<std::is_same<T, U>::value>,
            class   = typename optional_traits<T>::value_type>
  using enable_if_is_optional = T;

  template <class T>
  struct optional_traits<volatile enable_if_is_optional<T>> : public optional_traits<T> {};

  template <class T>
  struct optional_traits<const volatile enable_if_is_optional<const T>> : public optional_traits<const T> {};


  template <class V>
  struct optional_traits<std::optional<V>> {
    using value_type = V;
    using reference  = V&;
  };

  template <class V>
  struct optional_traits<const std::optional<V>> {
    using value_type = V;
    using reference  = const V&;
  };

  template <class, class = void>
  struct optional_traits_impl {};

  template <class T>
  struct optional_traits_impl<
        T,
        std::void_t<std::enable_if_t<std::is_convertible_v<T, bool>>, decltype(*std::declval<T&>())>> {
    using reference                   = decltype(*std::declval<T&>());
    using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
  };

  template <class T>
  struct optional_traits<
        T,
        std::enable_if_t<!is_tuple_v<T> && !std::is_array_v<T> && !std::is_void_v<std::remove_pointer_t<T>>>>
        : optional_traits_impl<T> {};
} // namespace impl

template <class, class = void>
constexpr inline bool is_optional_v = false;
template <class T>
constexpr inline bool is_optional_v<T, std::void_t<typename impl::optional_traits<T>::value_type>> = true;

} // namespace bxlx::graph::type_classification

#endif //BXLX_GRAPH_TYPE_CLASSIFICATION_HPP
