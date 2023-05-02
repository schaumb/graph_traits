//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_ASSERT_TYPES_HPP
#define BXLX_GRAPH_ASSERT_TYPES_HPP

#include <type_traits>

namespace bxlx {
template<class Why>
struct why_not_graph {
  static_assert(Why::value, "Not a graph, see under/below the reason");
};


template<class Reason, class ...>
struct reason : std::false_type {};


template<auto e, class ...>
struct expected;

template<auto type, class ...>
struct got;

template<class ...>
struct at;

template<class ...>
struct input;

struct expected_tuple;
struct multiple_good_recognition;
struct no_good_recognition;
struct not_random_access_range;
struct not_expected;
struct tuple_size_not_greater_than_2;
struct tuple_size_not_equal_to_2;
struct empty_tuple;
struct already_contains_property;

namespace graph::assert_types {
  using bxlx::why_not_graph;
  using bxlx::reason;
  using bxlx::expected;
  using bxlx::got;
  using bxlx::at;
  using bxlx::input;
}
}

#endif //BXLX_GRAPH_ASSERT_TYPES_HPP
