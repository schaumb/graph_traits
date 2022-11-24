//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <bxlx/graph/traits/graph_traits.hpp>
#include <set>
#include <map>
#include <vector>
#include <forward_list>
#include <optional>
#include <initializer_list>
#include <string>
#include <array>

template <class T>
using ra_range = std::vector<T>;
template <class T>
using range = std::forward_list<T>;
template <class T>
using set = std::set<T>;
template <class T, class U>
using map = std::map<T, U>;
template <class ...Ts>
using tup = std::tuple<Ts...>;
template <class T>
using opt = std::optional<T>;
using bool_t = bool;
using integral = std::size_t;
using string = std::string;
template <class T>
using fx_range = std::array<T, 5>;

using namespace bxlx::traits;

static_assert(!is_graph_v<void>);
static_assert(!is_graph_v<void*>);
static_assert(!is_graph_v<set<set<int>>>);
static_assert(!is_graph_v<ra_range<set<bool>>>);
static_assert(!is_graph_v<tup<set<bool>>>);
static_assert(!is_graph_v<set<tup<int, unsigned int>>>);

static_assert(is_graph_v<ra_range<ra_range<int>>>);
static_assert(is_graph_v<ra_range<set<int>>>);
static_assert(is_graph_v<ra_range<ra_range<tup<int, int>>>>);
static_assert(is_graph_v<ra_range<ra_range<bool>>>);

static_assert(is_graph_v<set<tup<int, int>>>);
static_assert(is_graph_v<set<tup<int, int, float>>>);
