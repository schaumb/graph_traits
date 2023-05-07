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

template <class T, bool possibly_optional, bool all_template_defined, class = void>
struct optional_traits_impl {};

template <class T>
struct optional_traits_impl<T,
                            true,
                            true,
                            std::void_t<decltype(static_cast<bool>(std::declval<T>())), decltype(*std::declval<T>())>> {
  using reference [[maybe_unused]]  = decltype(*std::declval<T&>());
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
};

struct defined_optional_value {
  constexpr bool operator<(const defined_optional_value&) const { return false; }
  constexpr bool operator==(const defined_optional_value&) const { return false; }
};

template<class NotDefined>
struct defined_value {
  using type = copy_cvref_t<NotDefined, defined_optional_value>;
};

template<class NotDefined>
using defined_value_t = typename defined_value<NotDefined>::type;

template<class NotDefined>
struct defined_value<NotDefined[]> {
  using type = defined_value_t<NotDefined>[];
};

template<class NotDefined, std::size_t N>
struct defined_value<NotDefined[N]> {
  using type = defined_value_t<NotDefined>[N];
};

template<class NotDefined>
struct defined_value<NotDefined*> {
  using type = defined_value_t<NotDefined>*;
};

template <template <class, class...> class Opt, class O, class...Oth>
struct optional_traits_impl<const Opt<O, Oth...>,
                            true,
                            false,
                            std::enable_if_t<!is_defined_v<O> && is_optional_v<const Opt<defined_value_t<O>>>>> {
  using reference [[maybe_unused]]  = typename replace_all_type_recursively<optional_reference_t<const Opt<defined_value_t<O>>>, defined_value_t<O>, O>::type;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
};

template <template <class, class...> class Opt, class O, class...Oth>
struct optional_traits_impl<Opt<O, Oth...>,
                            true,
                            false,
                            std::enable_if_t<!is_defined_v<O> && is_optional_v<Opt<defined_value_t<O>>>>> {
  using reference [[maybe_unused]]  = typename replace_all_type_recursively<optional_reference_t<Opt<defined_value_t<O>>>, defined_value_t<O>, O>::type;
  using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
};

template <class T>
struct optional_traits<T, true, std::enable_if_t<required_template_arguments_defined_v<T>>>
      : optional_traits_impl<T, !is_range_v<T>, true> {};

template <class T>
struct optional_traits<T, true, std::enable_if_t<!required_template_arguments_defined_v<T>>>
      : optional_traits_impl<T, true, false> {};

} // namespace bxlx::graph::type_traits::detail

namespace std {
template <>
struct hash<bxlx::graph::type_traits::detail::defined_optional_value> {
  constexpr std::size_t operator()(const bxlx::graph::type_traits::detail::defined_optional_value&) const { return {}; }
};
}
#endif //BXLX_GRAPH_OPTIONAL_TRAITS_HPP
