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
#include <list>
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
using si_range = std::list<T>;
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

using bxlx::traits::is_graph_v;
using bxlx::traits::graph_traits;
using bxlx::traits::graph_representation;

static_assert(!is_graph_v<void>);
static_assert(!is_graph_v<void*>);
static_assert(!is_graph_v<set<set<int>>>);
static_assert(!is_graph_v<ra_range<set<bool>>>);
static_assert(!is_graph_v<tup<set<bool>>>);
static_assert(!is_graph_v<set<tup<int, unsigned int>>>);

static_assert(is_graph_v<ra_range<set<int>>>);
static_assert(is_graph_v<set<tup<int, int>>>);
static_assert(is_graph_v<set<tup<int, int, float>>>);

template<class T>
constexpr static inline auto repr = graph_traits<T>::representation;

static_assert(repr<ra_range<set<int>>> == graph_representation::adjacency_list);
static_assert(repr<map<string, map<string, int>>> == graph_representation::adjacency_list);
static_assert(repr<ra_range<ra_range<bool>>> == graph_representation::adjacency_matrix);
static_assert(repr<set<tup<int, int>>> == graph_representation::edge_list);


template<class T, graph_representation repr, class node_repr_type, class edge_repr_type, std::size_t nodes, std::size_t edges,
    class graph_prop = void, class node_prop = void, class edge_prop = void>
constexpr static bool assert_on() {
    using traits = graph_traits<T>;
    static_assert(traits::representation == repr);
    static_assert(traits::has_graph_property == !std::is_void_v<graph_prop>);
    static_assert(traits::has_node_property == !std::is_void_v<node_prop>);
    static_assert(traits::has_edge_property == !std::is_void_v<edge_prop>);
    static_assert(traits::max_node_compile_time == nodes);
    static_assert(traits::max_edge_compile_time == edges);
    static_assert(std::is_same_v<node_repr_type, typename traits::node_repr_type>);
    static_assert(std::is_same_v<edge_repr_type, typename traits::edge_repr_type>);
    return true;
};

static_assert(assert_on<set<tup<int, int>>, graph_representation::edge_list, void*, tup<int, int>, 0, 0>());
static_assert(assert_on<set<tup<int, int, int>>, graph_representation::edge_list, void*, tup<int, int, int>, 0, 0, void, void, int>());
static_assert(assert_on<tup<fx_range<tup<int, int, struct XX>>, struct A>, graph_representation::edge_list, void*, tup<int, int, struct XX>, 10, 5, struct A, void, struct XX>());
static_assert(assert_on<tup<ra_range<tup<fx_range<opt<struct edge_prop>>, struct node_prop>>, struct graph_prop>,
    graph_representation::adjacency_matrix, tup<fx_range<opt<struct edge_prop>>, struct node_prop>,
    opt<struct edge_prop>, 5, 25, struct graph_prop, struct node_prop, struct edge_prop>());


static_assert(is_graph_v< ra_range<   range<integral>>>);
static_assert(is_graph_v< ra_range<ra_range<  bool_t>>>);
static_assert(is_graph_v< ra_range<ra_range<     opt<struct edge_prop>>>>);
static_assert(is_graph_v< ra_range<   range<     tup<integral, struct edge_prop>>>>);
static_assert(is_graph_v< si_range<     tup<integral, integral>>>);
static_assert(is_graph_v< si_range<     tup<integral, integral, struct edge_prop>>>);
static_assert(is_graph_v< ra_range<     tup<   range< integral>, struct node_prop>>>);
static_assert(is_graph_v< ra_range<     tup<ra_range<   bool_t>, struct node_prop>>>);
static_assert(is_graph_v< ra_range<     tup<ra_range<      opt<struct edge_prop>>, struct node_prop>>>);
static_assert(is_graph_v< ra_range<     tup<   range<      tup<integral, struct edge_prop>>, struct node_prop>>>);

static_assert(is_graph_v< tup<ra_range<range<integral>>, struct graph_prop>>);
static_assert(is_graph_v< tup<ra_range<ra_range<bool_t>>, struct graph_prop>>);
static_assert(is_graph_v< tup<si_range<tup<integral, integral>>, struct graph_prop>>);
static_assert(is_graph_v< tup<ra_range<range<tup<integral, struct edge_prop>>>, struct graph_prop>>);
static_assert(is_graph_v< tup<ra_range<ra_range<opt<struct edge_prop>>>, struct graph_prop>>);
static_assert(is_graph_v< tup<si_range<tup<integral, integral, struct edge_prop>>, struct graph_prop>>);
static_assert(is_graph_v< tup<ra_range<tup<range<integral>, struct node_prop>>, struct graph_prop>>);
static_assert(is_graph_v< tup<ra_range<tup<ra_range<bool_t>, struct node_prop>>, struct graph_prop>>);
static_assert(is_graph_v< tup<ra_range<tup<range<tup<integral, struct edge_prop>>, struct node_prop>>, struct graph_prop>>);
static_assert(is_graph_v< tup<ra_range<tup<ra_range<opt<struct edge_prop>>, struct node_prop>>, struct graph_prop>>);