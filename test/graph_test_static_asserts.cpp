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

template<class T>
struct type_identity { using type = T; };

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
    class graph_prop, class node_prop, class edge_prop, bool user_defined>
constexpr static bool assert_on() {
    static_assert(bxlx::traits::is_it_a_graph<T>);
    static_assert(is_graph_v<T>);

    using traits = graph_traits<T>;
    static_assert(traits::representation == repr);
    static_assert(user_defined == traits::user_node_index);
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
        (lambda(type_identity<Ts<T, U>>{}, ps...), ...);
    }
};

template<template<class> class ... Ts>
struct range_it {
    template<class T, class Lambda, class ...Ps>
    constexpr static void for_each(Lambda&& lambda, Ps&&... ps) {
        (lambda(type_identity<Ts<T>>{}, ps...), ...);
    }
};

template<class ...Ts>
struct type_holders {
    template<class Lambda, class ...Ps>
    constexpr static void for_each(Lambda&& lambda, Ps&&... ps) {
        (lambda(type_identity<Ts>{}, ps...), ...);
    }
};

template <class T, class U>
using other_map = std::unordered_map<T, U>;

using map_with_node_index = map_it<map, other_map>;
using node_indexed_ranges = range_it<fx_range, ra_range>;
using edge_ranges = range_it<fx_range, ra_range, set, si_range>;
using any_ranges = range_it<fx_range, range>;
using optionals = range_it<opt, std::add_pointer_t>;
using bools = type_holders<bool>;
using bitsets = type_holders<std::bitset<5>, std::vector<bool>>;
using node_indices = type_holders<std::string_view, int>;
using indices = type_holders<int>;
using props = type_holders<int>;

static_assert(std::is_same_v<graph_traits<std::initializer_list<std::pair<int, int>>>::node_index_t, int>);

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
            typename decltype(edge_prop)::type,
            false
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeRange = typename decltype(node_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        checker(type_identity<std::pair<NodeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_indexed_range = [&] (auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        } else {
            checker(node_range, type_identity<void>{}, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using Range = typename decltype(range)::type;
        using NodeProp = typename decltype(node_prop)::type;
        node_indexed_ranges::for_each<std::pair<Range, NodeProp>>(for_each_indexed_range, type_identity<std::pair<Range, NodeProp>>{}, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_any_range = [&] (auto range, auto edge_repr, auto edge_prop, auto index) {
        using Range = typename decltype(range)::type;
        if constexpr (node_p) {
            props::for_each(for_each_node_prop, range, edge_repr, edge_prop, index);
        } else {
            node_indexed_ranges::for_each<Range>(for_each_indexed_range, range, type_identity<void>{}, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_edge_prop = [&] (auto edge_prop, auto index) {
        using Index = typename decltype(index)::type;
        using EdgeProp = typename decltype(edge_prop)::type;
        any_ranges::for_each<std::pair<Index, EdgeProp>>(for_each_any_range, type_identity<std::pair<Index, EdgeProp>>{}, edge_prop, index);
    };

    auto for_each_indices = [&] (auto index) {
        using Index = typename decltype(index)::type;
        if constexpr (edge_p) {
            props::for_each(for_each_edge_prop, index);
        } else {
            any_ranges::for_each<Index>(for_each_any_range, index, type_identity<void>{}, index);
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
            typename decltype(edge_prop)::type,
            true
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeRange = typename decltype(node_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        checker(type_identity<std::pair<NodeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_indexed_range = [&] (auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        } else {
            checker(node_range, type_identity<void>{}, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeIndex = typename decltype(index)::type;
        using Range = typename decltype(range)::type;
        using NodeProp = typename decltype(node_prop)::type;
        map_with_node_index::for_each<NodeIndex, std::pair<Range, NodeProp>>(for_each_indexed_range, type_identity<
            std::pair<const NodeIndex, std::pair<Range, NodeProp>>>{}, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_any_range = [&] (auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeIndex = typename decltype(index)::type;
        using Range = typename decltype(range)::type;
        if constexpr (node_p) {
            props::for_each(for_each_node_prop, range, edge_repr, edge_prop, index);
        } else {
            map_with_node_index::for_each<NodeIndex, Range>(for_each_indexed_range, type_identity<std::pair<const NodeIndex, Range>>{}, type_identity<void>{}, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_edge_prop = [&] (auto edge_prop, auto index) {
        using Index = typename decltype(index)::type;
        using EdgeProp = typename decltype(edge_prop)::type;
        any_ranges::for_each<std::pair<Index, EdgeProp>>(for_each_any_range, type_identity<std::pair<Index, EdgeProp>>{}, edge_prop, index);
    };

    auto for_each_indices = [&] (auto index) {
        using Index = typename decltype(index)::type;
        if constexpr (edge_p) {
            props::for_each(for_each_edge_prop, index);
        } else {
            any_ranges::for_each<Index>(for_each_any_range, index, type_identity<void>{}, index);
        }
    };

    node_indices::for_each(for_each_indices);
    return true;
}


template<bool node_p, bool edge_p, bool graph_p>
constexpr bool check_all_adj_list_3() {
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
            typename decltype(edge_prop)::type,
            true
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeRange = typename decltype(node_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        checker(type_identity<std::pair<NodeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_indexed_range = [&] (auto node_range, auto node_repr, auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        } else {
            checker(node_range, type_identity<void>{}, node_range, node_repr, node_prop, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeIndex = typename decltype(index)::type;
        using Range = typename decltype(range)::type;
        using NodeProp = typename decltype(node_prop)::type;
        map_with_node_index::for_each<NodeIndex, std::pair<Range, NodeProp>>(for_each_indexed_range, type_identity<
            std::pair<const NodeIndex, std::pair<Range, NodeProp>>>{}, node_prop, range, edge_repr, edge_prop, index);
    };
    auto for_each_any_range = [&] (auto range, auto edge_repr, auto edge_prop, auto index) {
        using NodeIndex = typename decltype(index)::type;
        using Range = typename decltype(range)::type;
        if constexpr (node_p) {
            props::for_each(for_each_node_prop, range, edge_repr, edge_prop, index);
        } else {
            map_with_node_index::for_each<NodeIndex, Range>(for_each_indexed_range, type_identity<std::pair<const NodeIndex, Range>>{}, type_identity<void>{}, range, edge_repr, edge_prop, index);
        }
    };

    auto for_each_edge_prop = [&] (auto edge_prop, auto index) {
        using Index = typename decltype(index)::type;
        using EdgeProp = typename decltype(edge_prop)::type;
        map_with_node_index::for_each<Index, EdgeProp>(for_each_any_range, type_identity<std::pair<const Index, EdgeProp>>{}, edge_prop, index);
    };

    auto for_each_indices = [&] (auto index) {
        if constexpr (edge_p) {
            props::for_each(for_each_edge_prop, index);
        }
    };

    node_indices::for_each(for_each_indices);
    return true;
}


template<bool node_p, bool edge_p, bool graph_p>
constexpr bool check_all_adj_matrix_1() {
    auto checker = [](auto graph, auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto bitset) {
        constexpr auto nodes = bxlx::detail2::compile_time_size_v<typename decltype(node_range)::type>;
        constexpr auto bit_node = bxlx::detail2::compile_time_size_v<typename decltype(bitset)::type>;
        static_assert(!nodes || !bit_node || nodes == bit_node);
        constexpr auto node_size = std::max(nodes, bit_node);
        assert_on<
            typename decltype(graph)::type,
            graph_representation::adjacency_matrix,
            std::size_t,
            typename decltype(node_repr)::type,
            bxlx::detail2::subscript_operator_return<typename decltype(bitset)::type>,
            node_size,
            node_size * node_size,
            typename decltype(graph_prop)::type,
            typename decltype(node_prop)::type,
            void,
            false
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto bitset) {
        using NodeRange = typename decltype(node_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        checker(type_identity<std::pair<NodeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, bitset);
    };
    auto for_each_indexed_range = [&] (auto node_range, auto node_repr, auto node_prop, auto bitset) {
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, bitset);
        } else {
            checker(node_range, type_identity<void>{}, node_range, node_repr, node_prop, bitset);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto bitset) {
        using Bitset = typename decltype(bitset)::type;
        using NodeProp = typename decltype(node_prop)::type;
        node_indexed_ranges::for_each<std::pair<Bitset, NodeProp>>(
            for_each_indexed_range, type_identity<std::pair<Bitset, NodeProp>>{}, node_prop, bitset);
    };

    auto for_each_bitset = [&] (auto bitset) {
        using Bitset = typename decltype(bitset)::type;
        if constexpr (node_p) {
            props::for_each(for_each_node_prop, bitset);
        } else {
            node_indexed_ranges::for_each<Bitset>(for_each_indexed_range, bitset, type_identity<void>{}, bitset);
        }
    };

    if constexpr (!edge_p) {
        bitsets::for_each(for_each_bitset);
    }
    return true;
}


template<bool node_p, bool edge_p, bool graph_p>
constexpr bool check_all_adj_matrix_2() {
    auto checker = [](auto graph, auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto inside_range, auto edge_repr, auto edge_prop) {
        constexpr auto nodes = bxlx::detail2::compile_time_size_v<typename decltype(node_range)::type>;
        constexpr auto range_size = bxlx::detail2::compile_time_size_v<typename decltype(inside_range)::type>;
        constexpr auto node_size = std::max(nodes, range_size);
        assert_on<
            typename decltype(graph)::type,
            graph_representation::adjacency_matrix,
            std::size_t,
            typename decltype(node_repr)::type,
            typename decltype(edge_repr)::type,
            node_size,
            node_size * node_size,
            typename decltype(graph_prop)::type,
            typename decltype(node_prop)::type,
            typename decltype(edge_prop)::type,
            false
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto inside_range, auto edge_repr, auto edge_prop) {
        using NodeRange = typename decltype(node_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        checker(type_identity<std::pair<NodeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, inside_range, edge_repr, edge_prop);
    };
    auto for_each_indexed_range = [&] (auto node_range, auto node_repr, auto node_prop, auto inside_range, auto edge_repr, auto edge_prop) {
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, inside_range, edge_repr, edge_prop);
        } else {
            checker(node_range, type_identity<void>{}, node_range, node_repr, node_prop, inside_range, edge_repr, edge_prop);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto inside_range, auto edge_repr, auto edge_prop) {
        using InsideRange = typename decltype(inside_range)::type;
        using NodeProp = typename decltype(node_prop)::type;
        node_indexed_ranges::for_each<std::pair<InsideRange, NodeProp>>(
            for_each_indexed_range, type_identity<std::pair<InsideRange, NodeProp>>{}, node_prop, inside_range, edge_repr, edge_prop);
    };

    auto for_each_inside_indexed_range = [&] (auto inside_range, auto edge_repr, auto edge_prop) {
        using InsideRange = typename decltype(inside_range)::type;
        if constexpr (bxlx::detail2::classify<InsideRange> != bxlx::detail2::type_classification::bitset_like_container) {
            if constexpr (node_p) {
                props::for_each(for_each_node_prop, inside_range, edge_repr, edge_prop);
            } else {
                node_indexed_ranges::for_each<InsideRange>(for_each_indexed_range, inside_range, type_identity<void>{}, inside_range, edge_repr, edge_prop);
            }
        }
    };

    if constexpr (edge_p) {
        auto for_each_optional = [&] (auto edge_repr, auto edge_prop) {
            using EdgeRepr = typename decltype(edge_repr)::type;
            node_indexed_ranges::for_each<EdgeRepr>(for_each_inside_indexed_range, edge_repr, edge_prop);
        };

        props::for_each([&] (auto edge_prop) {
            using EdgeProp = typename decltype(edge_prop)::type;
            optionals::for_each<EdgeProp>(for_each_optional, edge_prop);
        });
    } else {
        bools::for_each([&] (auto bool_prop) {
            using BoolProp = typename decltype(bool_prop)::type;
            node_indexed_ranges::for_each<BoolProp>(for_each_inside_indexed_range, bool_prop, type_identity<void>{});
        });
    }
    return true;
}

template<bool node_p, bool edge_p, bool graph_p>
constexpr bool check_all_edge_list_1() {
    auto checker = [](auto graph, auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto edge_range, auto edge_repr, auto edge_prop, auto node_index) {
        constexpr auto node_size = bxlx::detail2::compile_time_size_v<typename decltype(node_range)::type>;
        constexpr auto edges_size = bxlx::detail2::compile_time_size_v<typename decltype(edge_range)::type>;
        assert_on<
            typename decltype(graph)::type,
            graph_representation::edge_list,
            typename decltype(node_index)::type,
            typename decltype(node_repr)::type,
            typename decltype(edge_repr)::type,
            node_size ? node_size : 2 * edges_size,
            edges_size,
            typename decltype(graph_prop)::type,
            typename decltype(node_prop)::type,
            typename decltype(edge_prop)::type,
            true
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto edge_range, auto edge_repr, auto edge_prop, auto node_index) {
        using NodeRange = typename decltype(node_range)::type;
        using EdgeRange = typename decltype(edge_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        if constexpr (std::is_void_v<NodeRange>) {
            checker(type_identity<std::pair<EdgeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, edge_range, edge_repr, edge_prop, node_index);
        } else {
            checker(type_identity<std::tuple<NodeRange, EdgeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, edge_range, edge_repr, edge_prop, node_index);
        }
    };

    auto for_each_node_map = [&] (auto node_range, auto node_prop, auto edge_range, auto edge_repr, auto edge_prop, auto node_index) {
        using NodeRange = typename decltype(node_range)::type;
        using EdgeRange = typename decltype(edge_range)::type;
        constexpr auto node_repr = type_identity<bxlx::detail2::range_traits_type<NodeRange>>{};
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, edge_range, edge_repr, edge_prop, node_index);
        } else {
            checker(type_identity<std::pair<NodeRange, EdgeRange>>{}, type_identity<void>{},
                    node_range, node_repr, node_prop, edge_range, edge_repr, edge_prop, node_index);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto edge_range, auto edge_repr, auto edge_prop, auto node_index) {
        using NodeProp = typename decltype(node_prop)::type;
        using NodeIndex = typename decltype(node_index)::type;
        map_with_node_index::for_each<NodeIndex, NodeProp>(for_each_node_map, node_prop, edge_range, edge_repr, edge_prop, node_index);
    };

    auto for_each_edge_range = [&] (auto edge_range, auto edge_prop, auto node_index) {
        constexpr auto edge_repr = type_identity<bxlx::detail2::range_traits_type<typename decltype(edge_range)::type>>{};
        if constexpr (node_p) {
            props::for_each(for_each_node_prop, edge_range, edge_repr, edge_prop, node_index);
        } else if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, type_identity<void>{}, type_identity<void>{}, type_identity<void>{}, edge_range, edge_repr, edge_prop, node_index);
        } else {
            checker(edge_range, type_identity<void>{}, type_identity<void>{}, type_identity<void>{}, type_identity<void>{}, edge_range, edge_repr, edge_prop, node_index);
        }
    };

    auto for_each_edge_prop = [&] (auto edge_prop, auto node_index) {
        using EdgeProp = typename decltype(edge_prop)::type;
        using NodeIndex = typename decltype(node_index)::type;
        edge_ranges::for_each<std::tuple<NodeIndex, NodeIndex, EdgeProp>>(for_each_edge_range, edge_prop, node_index);
    };

    node_indices::for_each([&] (auto node_index) {
        if constexpr (edge_p) {
            props::for_each(for_each_edge_prop, node_index);
        } else {
            using NodeIndex = typename decltype(node_index)::type;
            edge_ranges::for_each<std::pair<NodeIndex, NodeIndex>>(for_each_edge_range, type_identity<void>{}, node_index);
        }
    });
    return true;
}


template<bool node_p, bool edge_p, bool graph_p>
constexpr bool check_all_edge_list_2() {
    auto checker = [](auto graph, auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto edge_range, auto edge_repr, auto edge_prop, auto node_index) {
        constexpr auto node_size = bxlx::detail2::compile_time_size_v<typename decltype(node_range)::type>;
        constexpr auto edges_size = bxlx::detail2::compile_time_size_v<typename decltype(edge_range)::type>;
        assert_on<
            typename decltype(graph)::type,
            graph_representation::edge_list,
            typename decltype(node_index)::type,
            typename decltype(node_repr)::type,
            typename decltype(edge_repr)::type,
            node_size ? node_size : 2 * edges_size,
            edges_size,
            typename decltype(graph_prop)::type,
            typename decltype(node_prop)::type,
            typename decltype(edge_prop)::type,
            false
        >();
    };

    auto for_each_graph_prop = [&] (auto graph_prop, auto node_range, auto node_repr, auto node_prop, auto edge_range, auto edge_repr, auto edge_prop, auto node_index) {
        using NodeRange = typename decltype(node_range)::type;
        using EdgeRange = typename decltype(edge_range)::type;
        using GraphProp = typename decltype(graph_prop)::type;
        if constexpr (std::is_void_v<NodeRange>) {
            checker(type_identity<std::pair<EdgeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, edge_range, edge_repr, edge_prop, node_index);
        } else {
            checker(type_identity<std::tuple<NodeRange, EdgeRange, GraphProp>>{}, graph_prop, node_range, node_repr, node_prop, edge_range, edge_repr, edge_prop, node_index);
        }
    };

    auto for_each_node_map = [&] (auto node_range, auto node_repr, auto node_prop, auto edge_range, auto edge_repr, auto edge_prop, auto node_index) {
        using NodeRange = typename decltype(node_range)::type;
        using EdgeRange = typename decltype(edge_range)::type;
        if constexpr (graph_p) {
            props::for_each(for_each_graph_prop, node_range, node_repr, node_prop, edge_range, edge_repr, edge_prop, node_index);
        } else {
            checker(type_identity<std::pair<NodeRange, EdgeRange>>{}, type_identity<void>{},
                    node_range, node_repr, node_prop, edge_range, edge_repr, edge_prop, node_index);
        }
    };

    auto for_each_node_prop = [&] (auto node_prop, auto edge_range, auto edge_repr, auto edge_prop, auto node_index) {
        using NodeProp = typename decltype(node_prop)::type;
        node_indexed_ranges::for_each<NodeProp>(for_each_node_map, node_prop, node_prop, edge_range, edge_repr, edge_prop, node_index);
    };

    auto for_each_edge_range = [&] (auto edge_range, auto edge_prop, auto node_index) {
        if constexpr (node_p) {
            constexpr auto edge_repr = type_identity<bxlx::detail2::range_traits_type<typename decltype(edge_range)::type>>{};
            props::for_each(for_each_node_prop, edge_range, edge_repr, edge_prop, node_index);
        }
    };

    auto for_each_edge_prop = [&] (auto edge_prop, auto node_index) {
        using EdgeProp = typename decltype(edge_prop)::type;
        using NodeIndex = typename decltype(node_index)::type;
        edge_ranges::for_each<std::tuple<NodeIndex, NodeIndex, EdgeProp>>(for_each_edge_range, edge_prop, node_index);
    };

    indices::for_each([&] (auto node_index) {
        if constexpr (edge_p) {
            props::for_each(for_each_edge_prop, node_index);
        } else {
            using NodeIndex = typename decltype(node_index)::type;
            edge_ranges::for_each<std::pair<NodeIndex, NodeIndex>>(for_each_edge_range, type_identity<void>{}, node_index);
        }
    });
    return true;
}

using check_nodes = type_holders<
    bxlx::traits::constant_t<0>, bxlx::traits::constant_t<1>, bxlx::traits::constant_t<2>, bxlx::traits::constant_t<3>,
    bxlx::traits::constant_t<4>, bxlx::traits::constant_t<5>, bxlx::traits::constant_t<6>, bxlx::traits::constant_t<7>
>;

[[maybe_unused]]
constexpr auto ignore = (check_nodes::for_each([] (auto v) {
    constexpr auto Val = decltype(v)::type::value;
    static_assert(check_all_adj_list_1<!(Val & 1), !(Val & 2), !(Val & 4)>());
    static_assert(check_all_adj_list_2<!(Val & 1), !(Val & 2), !(Val & 4)>());
    static_assert(check_all_adj_list_3<!(Val & 1), !(Val & 2), !(Val & 4)>());
    static_assert(check_all_adj_matrix_1<!(Val & 1), !(Val & 2), !(Val & 4)>());
    static_assert(check_all_adj_matrix_2<!(Val & 1), !(Val & 2), !(Val & 4)>());
    static_assert(check_all_edge_list_1<!(Val & 1), !(Val & 2), !(Val & 4)>());
    static_assert(check_all_edge_list_2<!(Val & 1), !(Val & 2), !(Val & 4)>());
}), 0);

static_assert(assert_on<set<tup<int, int>>, graph_representation::edge_list, int, void, const tup<int, int>, 0, 0, void, void, void, true>());
static_assert(assert_on<set<tup<int, int, int>>, graph_representation::edge_list, int, void, const tup<int, int, int>, 0, 0, void, void, int, true>());
static_assert(assert_on<tup<fx_range<tup<int, int, struct XX>>, struct A>, graph_representation::edge_list, int, void, tup<int, int, struct XX>, 10, 5, struct A, void, struct XX, true>());
static_assert(assert_on<tup<ra_range<tup<fx_range<opt<struct edge_prop>>, struct node_prop>>, struct graph_prop>,
    graph_representation::adjacency_matrix, std::size_t, tup<fx_range<opt<struct edge_prop>>, struct node_prop>,
    opt<struct edge_prop>, 5, 25, struct graph_prop, struct node_prop, struct edge_prop, false>());


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