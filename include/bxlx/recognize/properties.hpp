//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_PROPERTIES_HPP
#define BXLX_GRAPH_PROPERTIES_HPP

#include <type_traits>

namespace bxlx::graph::traits::properties {
template <class>
struct Key {};

struct undefined;

template <class T = undefined>
struct Value {
  constexpr operator bool() const { return true; }
  using type = T;
};

template <>
struct Value<undefined> {
  constexpr operator bool() const { return false; }
};

constexpr Value<undefined> operator||(Value<undefined> const&, Value<undefined> const&) { return {}; }

template <class T>
constexpr Value<T> operator||(Value<T> const&, Value<undefined> const&) {
  return {};
}

template <class T>
constexpr Value<T> operator||(Value<undefined> const&, Value<T> const&) {
  return {};
}


template <class K, class V>
struct property {
  template <class Key2, class Value2>
  constexpr bool operator&&(property<Key2, Value2> const&) const {
    return !std::is_same_v<K, Key2> || std::is_same_v<V, Value2>;
  }

  template <class T>
  constexpr std::conditional_t<std::is_same_v<T, K>, Value<V>, Value<>> is_key(Key<T> const& = {}) const {
    return {};
  }
};


template <class... Ts>
struct properties {
  template <class Prop>
  constexpr bool is_valid(Prop const& = {}) const {
    return ((Prop{} && Ts{}) && ...);
  }

  constexpr operator bool() const { return (is_valid(Ts{}) && ...); }

  template <class... Us>
  constexpr properties<Ts..., Us...> operator+(properties<Us...> const&) const {
    return {};
  }

  template <class T>
  constexpr auto has_property(Key<T> const& e = {}) const {
    return (Ts{}.is_key(e) || ...);
  }
};


template <class K, class V>
constexpr inline properties<property<K, V>> property_v{};

constexpr inline properties<> empty_property_v{};

} // namespace bxlx::graph::traits::properties

#endif //BXLX_GRAPH_PROPERTIES_HPP
