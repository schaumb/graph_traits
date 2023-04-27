//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_CLASSIFY_HPP
#define BXLX_GRAPH_CLASSIFY_HPP

#include "optional_traits.hpp"
#include "range_traits.hpp"
#include "type_traits.hpp"

namespace bxlx::graph::classification {

enum class type {
  indeterminate,
  pre_declared,
  bitset,
  map_like,
  range,
  tuple_like,
  optional,
  bool_t,
  index,
};

template <class Class>
constexpr inline type classify = type_traits::is_bitset_v<Class>     ? type::bitset
                                 : type_traits::is_map_v<Class>      ? type::map_like
                                 : type_traits::is_range_v<Class>    ? type::range
                                 : type_traits::is_tuple_v<Class>    ? type::tuple_like
                                 : type_traits::is_optional_v<Class> ? type::optional
                                 : type_traits::is_bool_v<Class>     ? type::bool_t
                                 : type_traits::is_index_v<Class>    ? type::index
                                 : type_traits::is_defined_v<Class>  ? type::indeterminate
                                                                     : type::pre_declared;
} // namespace bxlx::graph::classification

#endif //BXLX_GRAPH_CLASSIFY_HPP
