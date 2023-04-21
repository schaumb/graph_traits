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
          class U = std::remove_cv_t<T>,
          class   = std::enable_if_t<std::is_same_v<T, U>>,
          class   = typename optional_traits<T>::value_type>
using enable_if_is_optional = T;

template <class T, bool any, bool any2>
struct optional_traits<volatile enable_if_is_optional<T>, any, any2> : optional_traits<T> {};

template <class T, bool any, bool any2>
struct optional_traits<const volatile enable_if_is_optional<const T>, any, any2> : optional_traits<const T> {};


template <class V, bool any>
struct optional_traits<std::optional<V>, any, true> {
  using value_type = V;
  using reference  = V&;
};

template <class V, bool any>
struct optional_traits<const std::optional<V>, any, true> {
  using value_type = V const;
  using reference  = V const&;
};

template <class V, bool any>
struct optional_traits<V*, any, true> {
  using value_type = V;
  using reference  = V&;
};

template <class V, bool any>
struct optional_traits<V* const, any, true> {
  using value_type = V;
  using reference  = V&;
};

template <class T, bool all_defined, class = void>
struct optional_traits_impl {};

template <class T>
struct optional_traits_impl<T,
                            true,
                            std::void_t<decltype(static_cast<bool>(std::declval<T>())), decltype(*std::declval<T>())>> {
  using reference [[maybe_unused]]  = decltype(*std::declval<T&>());
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
};

template <class T, bool r, bool = !is_range_v<T, r>>
struct optional_traits_impl_helper {};

template <class T>
struct optional_traits_impl_helper<T, true, true> : optional_traits_impl<T, true> {};

template <class T>
struct optional_traits_impl_helper<T, false, true> : optional_traits_impl<T, is_defined_v<T>> {};

template <class T, bool any>
struct optional_traits<T, any, true> : optional_traits_impl_helper<T, any> {};

} // namespace bxlx::graph::type_traits::detail

#endif //BXLX_GRAPH_OPTIONAL_TRAITS_HPP
