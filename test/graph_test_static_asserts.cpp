//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <bxlx/graph/traits.hpp>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <forward_list>
#include <optional>
#include <initializer_list>
#include <string>
#include <array>
#include <bitset>
#include <atomic>

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

using bxlx::is_graph_v;
using bxlx::graph_traits;
using bxlx::graph_representation;

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

template<class T>
using simplified_why_not_a_graph = typename decltype(bxlx::traits::graph::why_not<T>())::simplified;

static_assert(std::is_same_v<simplified_why_not_a_graph<int>,
    bxlx::not_matching_types>);
static_assert(std::is_same_v<simplified_why_not_a_graph<std::map<int, std::set<unsigned int>>>,
    bxlx::multiple_property_for<bxlx::traits::node_index, int, unsigned int>>);
static_assert(std::is_same_v<simplified_why_not_a_graph<std::pair<std::vector<std::vector<int>>, std::vector<std::pair<int, int>>>>,
    bxlx::graph_multiple_recognize>);

static_assert(bxlx::is_it_a_graph<std::vector<std::vector<int>>>);

template<class T, graph_representation repr, class node_index_t, class node_repr_type, class edge_repr_type, std::size_t nodes, std::size_t edges,
    class graph_prop = void, class node_prop = void, class edge_prop = void>
constexpr static bool assert_on() {
    static_assert(bxlx::traits::is_it_a_graph<T>);
    static_assert(is_graph_v<T>);

    using traits = graph_traits<T>;
    static_assert(traits::representation == repr);
    static_assert(std::is_same_v<typename traits::node_index_t, node_index_t>);
    static_assert(traits::has_graph_property == !std::is_void_v<graph_prop>);
    static_assert(traits::has_node_property == !std::is_void_v<node_prop>);
    static_assert(traits::has_edge_property == !std::is_void_v<edge_prop>);
    static_assert(traits::max_node_compile_time == nodes);
    static_assert(traits::max_edge_compile_time == edges);
    static_assert(std::is_same_v<node_repr_type, typename traits::node_repr_type>);
    static_assert(std::is_same_v<edge_repr_type, typename traits::edge_repr_type>);
    static_assert(std::is_same_v<typename traits::graph_property_type, graph_prop>);
    static_assert(std::is_same_v<typename traits::node_property_type, node_prop>);
    static_assert(std::is_same_v<typename traits::edge_property_type, edge_prop>);
    return true;
};

template<template<class, class> class ... Ts>
struct map_it {
    template<class T, class U, class Lambda, class ...Ps>
    constexpr static void for_each(Lambda&& lambda, Ps&&... ps) {
        (lambda(std::common_type<Ts<T, U>>{}, ps...), ...);
    }
};

template<template<class> class ... Ts>
struct range_it {
    template<class T, class Lambda, class ...Ps>
    constexpr static void for_each(Lambda&& lambda, Ps&&... ps) {
        (lambda(std::common_type<Ts<T>>{}, ps...), ...);
    }
};

template<class ...Ts>
struct type_holders {
    template<class Lambda, class ...Ps>
    constexpr static void for_each(Lambda&& lambda, Ps&&... ps) {
        (lambda(std::common_type<Ts>{}, ps...), ...);
    }
};

using map_with_node_index = map_it<map>;
using node_indexed_ranges = range_it<fx_range, ra_range>;
using edge_ranges = range_it<fx_range, ra_range, set, si_range>;
using any_ranges = range_it<fx_range, range>;
using optionals = range_it<opt, std::add_pointer_t>;
using bitsets = type_holders<std::bitset<5>, std::vector<bool>>;
using node_indices = type_holders<std::string_view, int>;
using indices = type_holders<int>;
using props = type_holders<map<int, int>>;

template<bool node_p, bool edge_p, bool graph_p>
constexpr bool check_all_adj_list_1() {
    auto checker = [](auto graph, auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        constexpr auto nodes = bxlx::detail2::compile_time_size_v<typename decltype(node_range)::type>;
        constexpr auto inside = bxlx::detail2::compile_time_size_v<typename decltype(range)::type>;
        assert_on<
            typename decltype(graph)::type,
            graph_representation::adjacency_list,
            typename decltype(index)::type,
            typename decltype(node_repr)::type,
            typename decltype(edge_repr)::type,
            nodes,
            nodes * (inside ? inside : nodes),
            typename decltype(graph_prop)::type,
            typename decltype(node_prop)::type,
            typename decltype(edge_prop)::type
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeRange = typename decltype(node_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        checker(std::common_type<std::pair<NodeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_indexed_range = [&] (auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        } else {
            checker(node_range, std::common_type<void>{}, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using Range = typename decltype(range)::type;
        using NodeProp = typename decltype(node_prop)::type;
        node_indexed_ranges::for_each<std::pair<Range, NodeProp>>(for_each_indexed_range, std::common_type<std::pair<Range, NodeProp>>{}, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_any_range = [&] (auto range, auto edge_repr, auto edge_prop, auto index) {
        using Range = typename decltype(range)::type;
        if constexpr (node_p) {
            props::for_each(for_each_node_prop, range, edge_repr, edge_prop, index);
        } else {
            node_indexed_ranges::for_each<Range>(for_each_indexed_range, range, std::common_type<void>{}, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_edge_prop = [&] (auto edge_prop, auto index) {
        using Index = typename decltype(index)::type;
        using EdgeProp = typename decltype(edge_prop)::type;
        any_ranges::for_each<std::pair<Index, EdgeProp>>(for_each_any_range, std::common_type<std::pair<Index, EdgeProp>>{}, edge_prop, index);
    };

    auto for_each_indices = [&] (auto index) {
        using Index = typename decltype(index)::type;
        if constexpr (edge_p) {
            props::for_each(for_each_edge_prop, index);
        } else {
            any_ranges::for_each<Index>(for_each_any_range, index, std::common_type<void>{}, index);
        }
    };

    indices::for_each(for_each_indices);
    return true;
}

template<bool node_p, bool edge_p, bool graph_p>
constexpr bool check_all_adj_list_2() {
    auto checker = [](auto graph, auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        constexpr auto nodes = bxlx::detail2::compile_time_size_v<typename decltype(node_range)::type>;
        constexpr auto inside = bxlx::detail2::compile_time_size_v<typename decltype(range)::type>;
        assert_on<
            typename decltype(graph)::type,
            graph_representation::adjacency_list,
            typename decltype(index)::type,
            typename decltype(node_repr)::type,
            typename decltype(edge_repr)::type,
            nodes,
            nodes * (inside ? inside : nodes),
            typename decltype(graph_prop)::type,
            typename decltype(node_prop)::type,
            typename decltype(edge_prop)::type
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeRange = typename decltype(node_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        checker(std::common_type<std::pair<NodeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_indexed_range = [&] (auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        } else {
            checker(node_range, std::common_type<void>{}, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeIndex = typename decltype(index)::type;
        using Range = typename decltype(range)::type;
        using NodeProp = typename decltype(node_prop)::type;
        map_with_node_index::for_each<NodeIndex, std::pair<Range, NodeProp>>(for_each_indexed_range, std::common_type<
            std::pair<const NodeIndex, std::pair<Range, NodeProp>>>{}, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_any_range = [&] (auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeIndex = typename decltype(index)::type;
        using Range = typename decltype(range)::type;
        if constexpr (node_p) {
            props::for_each(for_each_node_prop, range, edge_repr, edge_prop, index);
        } else {
            map_with_node_index::for_each<NodeIndex, Range>(for_each_indexed_range, std::common_type<std::pair<const NodeIndex, Range>>{}, std::common_type<void>{}, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_edge_prop = [&] (auto edge_prop, auto index) {
        using Index = typename decltype(index)::type;
        using EdgeProp = typename decltype(edge_prop)::type;
        any_ranges::for_each<std::pair<Index, EdgeProp>>(for_each_any_range, std::common_type<std::pair<Index, EdgeProp>>{}, edge_prop, index);
    };

    auto for_each_indices = [&] (auto index) {
        using Index = typename decltype(index)::type;
        if constexpr (edge_p) {
            props::for_each(for_each_edge_prop, index);
        } else {
            any_ranges::for_each<Index>(for_each_any_range, index, std::common_type<void>{}, index);
        }
    };

    node_indices::for_each(for_each_indices);
    return true;
}

static_assert(check_all_adj_list_1<false, false, false>());
static_assert(check_all_adj_list_1<false, false, true>());
static_assert(check_all_adj_list_1<false, true, false>());
static_assert(check_all_adj_list_1<true, false, false>());
static_assert(check_all_adj_list_1<true, true, false>());
static_assert(check_all_adj_list_1<true, false, true>());
static_assert(check_all_adj_list_1<false, true, true>());
static_assert(check_all_adj_list_1<true, true, true>());

static_assert(check_all_adj_list_2<false, false, false>());
static_assert(check_all_adj_list_2<false, false, true>());
static_assert(check_all_adj_list_2<false, true, false>());
static_assert(check_all_adj_list_2<true, false, false>());
static_assert(check_all_adj_list_2<true, true, false>());
static_assert(check_all_adj_list_2<true, false, true>());
static_assert(check_all_adj_list_2<false, true, true>());
static_assert(check_all_adj_list_2<true, true, true>());

static_assert(assert_on<set<tup<int, int>>, graph_representation::edge_list, int, void, tup<int, int>, 0, 0>());
static_assert(assert_on<set<tup<int, int, int>>, graph_representation::edge_list, int, void, tup<int, int, int>, 0, 0, void, void, int>());
static_assert(assert_on<tup<fx_range<tup<int, int, struct XX>>, struct A>, graph_representation::edge_list, int, void, tup<int, int, struct XX>, 10, 5, struct A, void, struct XX>());
static_assert(assert_on<tup<ra_range<tup<fx_range<opt<struct edge_prop>>, struct node_prop>>, struct graph_prop>,
    graph_representation::adjacency_matrix, std::size_t, tup<fx_range<opt<struct edge_prop>>, struct node_prop>,
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