//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_GRAPH_NODE_HPP
#define BXLX_GRAPH_TRAITS_GRAPH_NODE_HPP

#include "../traits.hpp"

namespace bxlx::traits::node {
    template<class Traits>
    using node_t = typename Traits::node_index_t;
    template<class Traits>
    using node_repr_t = typename Traits::node_repr_type;
    template<class Traits>
    using node_prop_t = typename Traits::node_property_type;
    template<class Traits>
    using node_container_t = typename Traits::node_container_type;
    template<class Traits>
    using out_edge_container_t = typename Traits::out_edge_container_type;

    template<class Traits>
    constexpr static bool adj_list_v = Traits::representation == traits::graph_representation::adjacency_list;
    template<class Traits>
    constexpr static bool adj_matrix_v = Traits::representation == traits::graph_representation::adjacency_matrix;
    template<class Traits>
    constexpr static bool edge_list_v = Traits::representation == traits::graph_representation::edge_list;


    template<class not_a_graph, class ... Args>
    constexpr std::enable_if_t<!is_graph_v<not_a_graph>, bool> has_node(const not_a_graph&, Args&&...) = delete;

    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class Eq = std::equal_to<node_t<GraphTraits>>>
    constexpr auto has_node(const Graph& g, const node_t<GraphTraits>& n, Eq&& eq = {})
        -> std::enable_if_t<std::is_void_v<node_repr_t<GraphTraits>> &&
                            std::is_invocable_r_v<bool, Eq, node_t<GraphTraits>, node_t<GraphTraits>>, bool>
    {
        for (auto&& e : GraphTraits::get_edges(g)) {
            if (eq(n, GraphTraits::edge_source(e)) || eq(n, GraphTraits::edge_target(e))) {
                return true;
            }
        }
        return false;
    }

    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto has_node(const Graph& g, const node_t<GraphTraits>& n)
        -> std::enable_if_t<!std::is_void_v<node_repr_t<GraphTraits>> &&
                            GraphTraits::user_node_index, bool>
    {
        auto&& nodes = GraphTraits::get_nodes(g);
        return nodes.find(n) != std::end(nodes);
    }

    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto has_node(const Graph& g, const node_t<GraphTraits>& n)
        -> std::enable_if_t<!std::is_void_v<node_repr_t<GraphTraits>> &&
                            !GraphTraits::user_node_index, bool>
    {
        if constexpr (GraphTraits::max_node_compile_time > 0) {
            constexpr auto node_size = static_cast<node_t<GraphTraits>>(GraphTraits::max_node_compile_time);
            if constexpr (!std::is_unsigned_v<node_t<GraphTraits>>) {
                return node_t<GraphTraits>(0) <= n && n < node_size;
            } else {
                return n < node_size;
            }
        } else {
            auto node_size = static_cast<node_t<GraphTraits>>(std::size(GraphTraits::get_nodes(g)));
            if constexpr (!std::is_unsigned_v<node_t<GraphTraits>>) {
                return node_t<GraphTraits>(0) <= n && n < node_size;
            } else {
                return n < node_size;
            }
        }
    }


    template<class not_a_graph, class ... Args>
    constexpr std::enable_if_t<!is_graph_v<detail2::remove_cvref_t<std::remove_pointer_t<detail2::remove_cvref_t<not_a_graph>>>>, bool> get_node(not_a_graph&&, Args&&...) = delete;

    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr decltype(auto) get_node_impl(Graph& g, const node_t<GraphTraits>& n)
    {
        auto&& nodes = GraphTraits::get_nodes(g);
        if constexpr (GraphTraits::user_node_index) {
            if (auto it = nodes.find(n); it != std::end(nodes))
                return std::addressof(*it);
        } else if (has_node(g, n)) {
            if constexpr (detail2::has_subscript_operator<node_container_t<GraphTraits>>) {
                return std::addressof(nodes[n]);
            } else {
                return std::addressof(*(std::begin(nodes) + n));
            }
        }

        return static_cast<node_repr_t<GraphTraits>*>(nullptr);
    }

    template<class Graph, class GraphTraits = graph_traits_t<std::remove_reference_t<Graph>>>
    constexpr auto get_node(Graph&& g, const node_t<GraphTraits>& n)
        -> bxlx::detail2::copy_reference_t<Graph, node_repr_t<GraphTraits>>
    {
        if (auto it = get_node_impl(g, n))
            return *it;

        throw std::out_of_range("Node not exists");
    }

    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto get_node(Graph* g, const node_t<GraphTraits>& n)
        -> node_repr_t<GraphTraits>*
    {
        return get_node_impl(*g, n);
    }


    template<class not_a_graph, class ... Args>
    constexpr std::enable_if_t<!is_graph_v<detail2::remove_cvref_t<std::remove_pointer_t<detail2::remove_cvref_t<not_a_graph>>>>, bool>
            get_node_property(not_a_graph&&, Args&&...) = delete;

    template<class Graph, class GraphTraits = graph_traits_t<std::remove_reference_t<Graph>>>
    constexpr auto get_node_property(Graph&& g, const node_t<GraphTraits>& n)
        -> bxlx::detail2::copy_reference_t<Graph, node_prop_t<GraphTraits>>
    {
        return GraphTraits::get_node_property(get_node(std::forward<Graph>(g), n));
    }

    template<class Graph, class GraphTraits = graph_traits_t<std::remove_reference_t<Graph>>>
    constexpr auto get_node_property(Graph* g, const node_t<GraphTraits>& n)
        -> node_prop_t<GraphTraits>*
    {
        if (auto it = get_node_impl(*g, n))
            return std::addressof(GraphTraits::get_node_property(*it));
        return nullptr;
    }

    template<auto>
    constexpr static inline traits::identity_t pass{};

    template<class T, class With, std::size_t ...Ix>
    constexpr auto aggregate_initialize(std::index_sequence<Ix...>, const With& with) -> decltype(T{pass<Ix>(with)...}) {
        return T{pass<Ix>(with)...};
    }

    template<class, class, std::size_t, class = void>
    constexpr static bool is_aggregate_initializable_v = false;
    template<class T, class With, std::size_t N>
    constexpr static bool is_aggregate_initializable_v<T, With, N, std::void_t<
        decltype(aggregate_initialize<T, With>(std::make_index_sequence<N>{}, std::declval<const With&>()))
    >> = true;


    template<class GraphTraits, bool user_node_index = GraphTraits::user_node_index
                              , bool has_node_property = GraphTraits::has_node_property>
    struct add_node_traits;

    template<class GraphTraits>
    struct add_node_traits<GraphTraits, false, false> {
        template<class Traits = GraphTraits, bool = GraphTraits::out_edge_container_size != 0, class = void>
        struct check_emplace : std::false_type {};

        template<class Traits, bool fix_out_edge>
        struct check_emplace<Traits, fix_out_edge, std::enable_if_t<
            ((adj_list_v<Traits> && !fix_out_edge) || (adj_matrix_v<Traits> && fix_out_edge)) &&
            detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>>
        >> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back();
                return ix;
            }
        };

        template<class Traits>
        struct check_emplace<Traits, false, std::enable_if_t<
            adj_matrix_v<Traits> &&
            detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>> &&
            detail2::range_member_traits::has_resize_v<out_edge_container_t<Traits>>
        >> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back();
                std::size_t to = std::size(nodes);
                for (auto&& node : nodes)
                    Traits::out_edges(node).resize(to);
                return ix;
            }
        };

        template<class Traits>
        struct check_emplace<Traits, true, std::enable_if_t<
            adj_list_v<Traits> && !Traits::has_edge_property &&
            is_aggregate_initializable_v<out_edge_container_t<Traits>, node_t<Traits>, Traits::out_edge_container_size> &&
            detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>, out_edge_container_t<Traits>&&>
        >> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back(aggregate_initialize<
                    out_edge_container_t<Traits>
                >(std::make_index_sequence<Traits::out_edge_container_size>{}, Traits::invalid));
                return ix;
            }
        };

        template<class Traits>
        struct check_emplace<Traits, true, std::enable_if_t<
            adj_list_v<Traits> && Traits::has_edge_property &&
            is_aggregate_initializable_v<out_edge_container_t<Traits>, typename Traits::edge_repr_type, Traits::out_edge_container_size> &&
            detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>, out_edge_container_t<Traits>&&>
        >> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back(aggregate_initialize<
                    out_edge_container_t<Traits>
                >(std::make_index_sequence<Traits::out_edge_container_size>{}, typename Traits::edge_repr_type{Traits::invalid, {}}));
                return ix;
            }
        };
    };


    template<class GraphTraits>
    struct add_node_traits<GraphTraits, false, true> {
        template<class Traits, bool, class, class ...Args>
        struct check_emplace_impl : std::false_type {};

        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, false, std::enable_if_t<
            edge_list_v<Traits> &&
            detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>, Args...>
        >, Args...> : std::true_type {
            template<class Graph>
            [[maybe_unused]] constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back(std::forward<Args>(args)...);
                return ix;
            }
        };


        template<class Traits, bool fix_out_edge, class ...Args>
        struct check_emplace_impl<Traits, fix_out_edge, std::enable_if_t<
            ((adj_list_v<Traits> && !fix_out_edge) || (adj_matrix_v<Traits> && fix_out_edge)) &&
            std::is_constructible_v<node_repr_t<Traits>, std::piecewise_construct_t, std::tuple<>, std::tuple<Args&&...>> &&
            bxlx::detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>, std::piecewise_construct_t, std::tuple<>, std::tuple<Args&&...>>
        >, Args...> : std::true_type {
            template<class Graph>
            [[maybe_unused]] constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back(std::piecewise_construct, std::tuple<>{}, std::forward_as_tuple(std::forward<Args>(args)...));
                return ix;
            }
        };


        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, false, std::enable_if_t<
            adj_matrix_v<Traits> &&
            std::is_constructible_v<node_repr_t<Traits>, std::piecewise_construct_t, std::tuple<>, std::tuple<Args&&...>> &&
            bxlx::detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>, std::piecewise_construct_t, std::tuple<>, std::tuple<Args&&...>> &&
            bxlx::detail2::range_member_traits::has_resize_v<out_edge_container_t<Traits>>
        >, Args...> : std::true_type {
            template<class Graph>
            [[maybe_unused]] constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back(std::piecewise_construct, std::tuple<>{}, std::forward_as_tuple(std::forward<Args>(args)...));
                auto to = std::size(nodes);
                for (auto&& node : nodes)
                    Traits::out_edges(node).resize(to);
                return ix;
            }
        };

        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, true, std::enable_if_t<
            adj_list_v<Traits> && !Traits::has_edge_property &&
            std::is_constructible_v<node_repr_t<Traits>, std::piecewise_construct_t,
                std::tuple<out_edge_container_t<Traits>&&>, std::tuple<Args&&...>> &&
            is_aggregate_initializable_v<
                out_edge_container_t<Traits>,
                node_t<Traits>,
                Traits::out_edge_container_size
            > &&
            bxlx::detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>, std::piecewise_construct_t,
                std::tuple<out_edge_container_t<Traits>&&>, std::tuple<Args&&...>>
        >, Args...> : std::true_type {
            template<class Graph>
            [[maybe_unused]] constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back(std::piecewise_construct,
                                   std::forward_as_tuple(aggregate_initialize<
                                       out_edge_container_t<Traits>
                                   >(std::make_index_sequence<Traits::out_edge_container_size>{}, Traits::invalid)),
                                   std::forward_as_tuple(std::forward<Args>(args)...));
                return ix;
            }
        };


        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, true, std::enable_if_t<
            adj_list_v<Traits> && Traits::has_edge_property &&
            std::is_constructible_v<node_repr_t<Traits>, std::piecewise_construct_t,
                std::tuple<out_edge_container_t<Traits>&&>, std::tuple<Args&&...>> &&
            is_aggregate_initializable_v<
                out_edge_container_t<Traits>,
                typename Traits::edge_repr_type,
                Traits::out_edge_container_size
            > &&
            bxlx::detail2::range_member_traits::has_emplace_back_v<node_container_t<Traits>, std::piecewise_construct_t,
                std::tuple<out_edge_container_t<Traits>&&>, std::tuple<Args&&...>>
        >, Args...> : std::true_type {
            template<class Graph>
            [[maybe_unused]] constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::get_nodes(g);
                node_t<Traits> ix = std::size(nodes);
                nodes.emplace_back(std::piecewise_construct,
                                   std::forward_as_tuple(aggregate_initialize<
                                       out_edge_container_t<Traits>
                                   >(std::make_index_sequence<Traits::out_edge_container_size>{}, typename Traits::edge_repr_type{Traits::invalid, {}})),
                                   std::forward_as_tuple(std::forward<Args>(args)...));
                return ix;
            }
        };

        template<class ...Args>
        using check_emplace = check_emplace_impl<GraphTraits, GraphTraits::out_edge_container_size != 0, void, Args...>;
    };

    template<class GraphTraits>
    struct add_node_traits<GraphTraits, true, false> {
        template<class Traits = GraphTraits, bool = Traits::out_edge_container_size != 0, class = void>
        struct check_emplace_impl : std::false_type {};

        template<class Traits>
        struct check_emplace_impl<Traits, false, std::enable_if_t<
            adj_list_v<Traits> &&
            bxlx::detail2::range_member_traits::has_try_emplace_v<node_container_t<Traits>>
        >> : std::true_type {
            template<class Graph>
            [[maybe_unused]] constexpr static decltype(auto) do_it(Graph& g, const node_t<Traits>& i) {
                return std::make_pair(i, Traits::get_nodes(g).try_emplace(i).second);
            }
        };

        template<class...>
        using check_emplace = check_emplace_impl<>;
    };

    template<class GraphTraits>
    struct add_node_traits<GraphTraits, true, true> {
        template<class Traits, class, class ...Args>
        struct check_emplace_impl : std::false_type {};

        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, std::enable_if_t<
            edge_list_v<Traits> &&
            bxlx::detail2::range_member_traits::has_try_emplace_v<node_container_t<Traits>, Args...>
        >, Args...> : std::true_type {
            template<class Graph>
            [[maybe_unused]] constexpr static decltype(auto) do_it(Graph& g, const node_t<Traits>& i, Args&&...args) {
                return std::make_pair(i, Traits::get_nodes(g).try_emplace(i, std::forward<Args>(args)...).second);
            }
        };


        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, std::enable_if_t<
            adj_list_v<Traits> &&
            std::is_constructible_v<std::tuple_element_t<1, node_repr_t<Traits>>, std::piecewise_construct_t, std::tuple<>, std::tuple<Args&&...>> &&
            bxlx::detail2::range_member_traits::has_try_emplace_v<node_container_t<Traits>, std::piecewise_construct_t, std::tuple<>, std::tuple<Args&&...>>
        >, Args...> : std::true_type {
            template<class Graph>
            [[maybe_unused]] constexpr static decltype(auto) do_it(Graph& g, const node_t<Traits>& i, Args&&...args) {
                return std::make_pair(i, Traits::get_nodes(g).try_emplace(i, std::piecewise_construct, std::tuple<>{}, std::forward_as_tuple(std::forward<Args>(args)...)).second);
            }
        };

        template<class ...Args>
        using check_emplace = check_emplace_impl<GraphTraits, void, Args...>;
    };

    // not a graph
    template<class not_a_graph, class ... Args>
    constexpr std::enable_if_t<!is_graph_v<detail2::remove_cvref_t<not_a_graph>>, std::size_t> add_node(not_a_graph&, Args&&...) = delete;

    // fix node container type
    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class ... Args>
    constexpr std::enable_if_t<GraphTraits::node_container_size != 0, node_t<GraphTraits>> add_node(Graph&, Args&&...) = delete;

    // no node container
    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class ... Args>
    constexpr std::enable_if_t<std::is_void_v<typename GraphTraits::node_container_type>, node_t<GraphTraits>> add_node(Graph&, Args&&...) = delete;


    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto add_node(Graph& g)
        -> std::enable_if_t<!GraphTraits::user_node_index && !GraphTraits::has_node_property &&
            add_node_traits<GraphTraits>::template check_emplace<>::value
        , node_t<GraphTraits>>
    {
        return add_node_traits<GraphTraits>::template check_emplace<>::do_it(g);
    }

    // emplace_back() or matrix out_edges .resize() is not exist or adj_list edge_property not default constructible
    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto add_node(Graph& g)
        -> std::enable_if_t<!GraphTraits::user_node_index && !GraphTraits::has_node_property &&
                        !add_node_traits<GraphTraits>::template check_emplace<>::value
        , node_t<GraphTraits>> = delete;

    // for this graph, no argument accepted on add_node
    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class Arg, class ...Args>
    constexpr auto add_node(Graph& g, Arg&&, Args&&...)
        -> std::enable_if_t<!GraphTraits::user_node_index && !GraphTraits::has_node_property, node_t<GraphTraits>> = delete;


    // not creatable node property from Args, or not expandable graph
    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class ...Args>
    constexpr auto add_node(Graph& g, Args&&...)
        -> std::enable_if_t<!GraphTraits::user_node_index && GraphTraits::has_node_property &&
                        !add_node_traits<GraphTraits>::template check_emplace<Args&&...>::value
        , node_t<GraphTraits>> = delete;


    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class ...Args>
    constexpr auto add_node(Graph& g, Args&&... args)
        -> std::enable_if_t<!GraphTraits::user_node_index && GraphTraits::has_node_property &&
                            add_node_traits<GraphTraits>::template check_emplace<Args&&...>::value &&
                            std::is_constructible_v<node_prop_t<GraphTraits>, Args&&...>
        , node_t<GraphTraits>> {
        return add_node_traits<GraphTraits>::template check_emplace<Args&&...>::do_it(g, std::forward<Args>(args)...);
    };


    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto add_node(Graph& g, const node_t<GraphTraits>& n)
        -> std::enable_if_t<GraphTraits::user_node_index && !GraphTraits::has_node_property &&
                        add_node_traits<GraphTraits>::template check_emplace<>::value
        , std::pair<node_t<GraphTraits>, bool>> {
        return add_node_traits<GraphTraits>::template check_emplace<>::do_it(g, n);
    };

    // emplace() is not exist or adj_list edge_property not default constructible
    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto add_node(Graph& g, const node_t<GraphTraits>& n)
        -> std::enable_if_t<GraphTraits::user_node_index && !GraphTraits::has_node_property &&
                        !add_node_traits<GraphTraits>::template check_emplace<>::value
        , std::pair<node_t<GraphTraits>, bool>> = delete;

    // for this graph, must be exactly 1 argument required
    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto add_node(Graph& g)
        -> std::enable_if_t<GraphTraits::user_node_index && !GraphTraits::has_node_property, std::pair<node_t<GraphTraits>, bool>> = delete;

    // for this graph, must be exactly 1 argument required
    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class Arg, class ...Args>
    constexpr auto add_node(Graph& g, const node_t<GraphTraits>& n, Arg&&, Args&&...)
        -> std::enable_if_t<GraphTraits::user_node_index && !GraphTraits::has_node_property, std::pair<node_t<GraphTraits>, bool>> = delete;



    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class ...Args>
    constexpr auto add_node(Graph& g, const node_t<GraphTraits>& n, Args&&... args)
        -> std::enable_if_t<GraphTraits::user_node_index && GraphTraits::has_node_property &&
                        add_node_traits<GraphTraits>::template check_emplace<Args&&...>::value &&
                        std::is_constructible_v<node_prop_t<GraphTraits>, Args&&...>
        , std::pair<node_t<GraphTraits>, bool>> {
        return add_node_traits<GraphTraits>::template check_emplace<Args&&...>::do_it(g, n, std::forward<Args>(args)...);
    };

    // emplace() is not exist or adj_list edge_property not default constructible
    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class ...Args>
    constexpr auto add_node(Graph& g, const node_t<GraphTraits>& n, Args&&... args)
        -> std::enable_if_t<GraphTraits::user_node_index && GraphTraits::has_node_property &&
                        !add_node_traits<GraphTraits>::template check_emplace<Args&&...>::value
        , std::pair<node_t<GraphTraits>, bool>> = delete;

    // for this graph, at least 1 argument required
    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr auto add_node(Graph& g)
        -> std::enable_if_t<GraphTraits::user_node_index && GraphTraits::has_node_property, std::pair<node_t<GraphTraits>, bool>> = delete;

    /*
    template<class, class, class = void>
    constexpr static inline bool has_erase_v = false;
    template<class T, class K>
    constexpr static inline bool has_erase_v<T, K, std::void_t<
        decltype(detail2::member_function_invoke_result_v<decltype(std::size(std::declval<T&>())), T, K>(&T::erase))
    >> = true;

    template<class Graph, class GraphTraits = graph_traits<Graph>>
    constexpr std::size_t remove_node_from_edges(Graph& g, const node_t<GraphTraits>& i) {
        using Out = typename GraphTraits::out_edge_container_type;
        if constexpr (std::is_void_v<Out>) {
            using Edges = typename GraphTraits::edge_container_type;
            if constexpr (has_erase_v<Edges, detail2::get_begin_iterator_t<const Edges>>) {
                std::size_t res{};
                auto& edges = GraphTraits::get_edges(g);
                for (auto it = std::begin(edges), end = std::end(edges); it != end;) {
                    if (GraphTraits::edge_source(*it) == i || GraphTraits::edge_target(*it) == i) {
                        it = edges.erase(it); ++res;
                    } else ++it;
                }
                return res;
            }
        } else {

        }
        return 0;
    }

    template<class Graph, class GraphTraits = graph_traits<Graph>>
    constexpr std::size_t remove_node_from_edges(Graph&, const node_t<GraphTraits>&);

    template<class Graph, class GraphTraits = graph_traits<Graph>, class Eq = std::equal_to<node_t<GraphTraits>>>
    constexpr std::size_t remove_node_from_edges(Graph&, const node_t<GraphTraits>&, Eq&& = {});

     .erase(key)

     remove_if --> is_movable
     .resize() || adj_list && invalid


    template<class Graph, class GraphTraits = graph_traits<Graph>>
    constexpr bool remove_node(Graph&, const node_t<GraphTraits>&);
*/
}

#endif // BXLX_GRAPH_TRAITS_GRAPH_NODE_HPP
