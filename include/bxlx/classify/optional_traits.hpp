//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_OPTIONAL_TRAITS_HPP
#define BXLX_GRAPH_OPTIONAL_TRAITS_HPP

#include "type_traits.hpp"
#include <optional>

namespace bxlx::graph::type_traits::detail {

template <class T,
          class U = std::remove_volatile_t<T>,
          class   = std::enable_if_t<std::is_same_v<T, U>>,
          class   = typename known_optional<T>::value_type>
using enable_if_is_known_optional = T;

template <class T>
struct known_optional<volatile enable_if_is_known_optional<T>> : known_optional<T> {};

template <class T>
struct known_optional<const volatile enable_if_is_known_optional<const T>> : known_optional<const T> {};


template <class V>
struct known_optional<std::optional<V>> {
  using value_type = V;
  using reference  = V&;
};

template <class V>
struct known_optional<const std::optional<V>> {
  using value_type = V const;
  using reference  = V const&;
};

template <class V>
struct known_optional<V*, std::enable_if_t<!std::is_void_v<V>>> {
  using value_type = V;
  using reference  = V&;
};

template <class V>
struct known_optional<V* const, std::enable_if_t<!std::is_void_v<V>>> {
  using value_type = V;
  using reference  = V&;
};

template <class T, bool any>
struct optional_traits<T, any, std::enable_if_t<is_known_optional_v<T>>> : known_optional<T> {};


template <class T, bool all_defined, class = void>
struct optional_traits_impl {};

template <class T>
struct optional_traits_impl<T,
                            true,
                            std::void_t<decltype(static_cast<bool>(std::declval<T>())), decltype(*std::declval<T>())>> {
  using reference [[maybe_unused]]  = decltype(*std::declval<T&>());
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
};

template <class T>
struct optional_traits<T, true, std::enable_if_t<!is_known_optional_v<T>>> : optional_traits_impl<T, !is_known_range_v<T>> {};

} // namespace bxlx::graph::type_traits::detail

#endif //BXLX_GRAPH_OPTIONAL_TRAITS_HPP
