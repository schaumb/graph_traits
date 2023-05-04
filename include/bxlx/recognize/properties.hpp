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
struct Value : std::true_type {};

template <>
struct Value<undefined> : std::false_type {};

template<class ... Vals>
struct get_dominant_value {
  constexpr static std::size_t index = [] {
      bool arr[] {Vals::value...};
      std::size_t r{};
      for (; r+1 < sizeof...(Vals); ++r)
        if (arr[r])
          break;
      return r;
  } ();
  using type = std::tuple_element_t<index, std::tuple<Vals...>>;
};
template<>
struct get_dominant_value<> {
  using type = Value<>;
};

template <class K, class V>
struct property {

  template <class T>
  using value_if_key = std::conditional_t<std::is_same_v<T, K>, Value<V>, Value<>>;
};

template<class ...Ts>
struct properties;

template<class, class...>
struct merge_properties;

template<class ... Ts, class ... Us>
struct merge_properties<properties<Ts...>, properties<Us...>> {
  using type = properties<Ts..., Us...>;
};
template<class ... Ts, class ... Us, class ... Others>
struct merge_properties<properties<Ts...>, properties<Us...>, Others...> :
      merge_properties<properties<Ts..., Us...>, Others...>{};

template<class ...Ts>
using merge_properties_t = typename merge_properties<Ts...>::type;

template <class... Ts>
struct properties {
  template <class T>
  constexpr static auto has_property_v = typename get_dominant_value<typename Ts::template value_if_key<T>...>::type{};

  template <class T, class V>
  constexpr static bool is_valid_v = !has_property_v<T>() || std::is_same_v<std::decay_t<decltype(has_property_v<T>)>, Value<V>>;
};

template <class K, class V>
using property_t = properties<property<K, V>>;

using empty_t = properties<>;

} // namespace bxlx::graph::traits::properties

#endif //BXLX_GRAPH_PROPERTIES_HPP
