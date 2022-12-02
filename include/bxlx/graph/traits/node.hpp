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
                            GraphTraits::user_node_index>
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
            if constexpr (detail2::has_subscript_operator<std::remove_reference_t<decltype(nodes)>>) {
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

    template<typename T, typename With, std::size_t ...Ix>
    constexpr auto aggregate_initialize(std::index_sequence<Ix...>, const With& with) -> decltype(T{pass<Ix>(with)...}) {
        return T{pass<Ix>(with)...};
    }

    template<typename T, typename With, std::size_t N, class = void>
    constexpr static bool is_aggregate_initializable_v = false;
    template<typename T, typename With, std::size_t N>
    constexpr static bool is_aggregate_initializable_v<T, With, N, std::void_t<
        decltype(aggregate_initialize<T, With>(std::make_index_sequence<N>{}, std::declval<const With&>()))
    >> = true;


    template<class GraphTraits, bool user_node_index = GraphTraits::user_node_index
                              , bool has_node_property = GraphTraits::has_node_property>
    struct add_node_traits;

    template<class GraphTraits>
    struct add_node_traits<GraphTraits, false, false> {
        using RealTraits = GraphTraits;
        constexpr static bool adj_list = GraphTraits::representation == traits::graph_representation::adjacency_list;
        constexpr static bool adj_matrix = GraphTraits::representation == traits::graph_representation::adjacency_matrix;
        constexpr static bool fix_out_edge = GraphTraits::out_edge_container_size != 0;
        using NodeContainer = typename GraphTraits::node_container_type;
        constexpr static auto node_size = decltype(std::size(std::declval<NodeContainer&>())){};
        using OutEdgeContainer = typename GraphTraits::out_edge_container_type;
        constexpr static auto out_edge_size = decltype(std::size(std::declval<OutEdgeContainer&>())){};
        constexpr static bool has_edge_prop = GraphTraits::has_edge_property;
        constexpr static auto out_edges = GraphTraits::out_edge_container_size;

        template<class Traits = add_node_traits, class = void>
        struct check_emplace : std::false_type {};

        template<class Traits>
        struct check_emplace<Traits, std::enable_if_t<
            (Traits::adj_list && !Traits::fix_out_edge) || (Traits::adj_matrix && Traits::fix_out_edge),
            std::void_t<decltype(std::declval<typename Traits::NodeContainer&>().emplace_back())>
        >> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back();
                return ix;
            }
        };

        template<class Traits>
        struct check_emplace<Traits, std::enable_if_t<
            (Traits::adj_matrix && !Traits::fix_out_edge),
            std::void_t<
                decltype(std::declval<typename Traits::NodeContainer&>().emplace_back()),
                decltype(std::declval<typename Traits::OutEdgeContainer>().resize(Traits::out_edge_size))
            >
        >> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back();
                auto to = std::size(nodes);
                for (auto&& node : nodes)
                    Traits::RealTraits::out_edges(node).resize(to);
                return ix;
            }
        };

        template<class Traits>
        struct check_emplace<Traits, std::enable_if_t<
            (Traits::adj_list && Traits::fix_out_edge && !Traits::has_edge_prop),
            std::enable_if_t<
                is_aggregate_initializable_v<
                    typename Traits::OutEdgeContainer,
                    typename Traits::RealTraits::node_index_t,
                    Traits::out_edges
                >,
                std::void_t<decltype(std::declval<typename Traits::NodeContainer&>().emplace_back(std::declval<typename Traits::OutEdgeContainer&&>()))>
            >
        >> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back(aggregate_initialize<
                    typename Traits::OutEdgeContainer
                >(std::make_index_sequence<Traits::out_edges>{}, Traits::RealTraits::invalid));
                return ix;
            }
        };

        template<class Traits>
        struct check_emplace<Traits, std::enable_if_t<
            (Traits::adj_list && Traits::fix_out_edge && Traits::has_edge_prop),
            std::enable_if_t<
                is_aggregate_initializable_v<
                    typename Traits::OutEdgeContainer,
                    typename Traits::RealTraits::edge_repr_type,
                    Traits::out_edges
                >,
                std::void_t<decltype(std::declval<typename Traits::NodeContainer&>().emplace_back(std::declval<typename Traits::OutEdgeContainer&&>()))>
            >
        >> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back(aggregate_initialize<
                    typename Traits::OutEdgeContainer
                >(std::make_index_sequence<Traits::out_edges>{}, typename Traits::RealTraits::edge_repr_type{Traits::RealTraits::invalid, {}}));
                return ix;
            }
        };
    };


    template<class GraphTraits>
    struct add_node_traits<GraphTraits, false, true> {
        using RealTraits = GraphTraits;
        constexpr static bool adj_list = GraphTraits::representation == traits::graph_representation::adjacency_list;
        constexpr static bool adj_matrix =
            GraphTraits::representation == traits::graph_representation::adjacency_matrix;
        constexpr static bool edge_list =
            GraphTraits::representation == traits::graph_representation::edge_list;
        constexpr static bool fix_out_edge = GraphTraits::out_edge_container_size != 0;
        using NodeContainer = typename GraphTraits::node_container_type;
        constexpr static auto node_size = decltype(std::size(std::declval<NodeContainer &>())){};
        using OutEdgeContainer = typename GraphTraits::out_edge_container_type;
        constexpr static bool has_edge_prop = GraphTraits::has_edge_property;
        constexpr static auto out_edges = GraphTraits::out_edge_container_size;

        template<class Traits, class, class ...Args>
        struct check_emplace_impl : std::false_type {};

        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, std::enable_if_t<
            Traits::edge_list,
            std::void_t<decltype(std::declval<typename Traits::NodeContainer&>().emplace_back(std::declval<Args&&>()...))>
        >, Args...> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back(std::forward<Args>(args)...);
                return ix;
            }
        };


        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, std::enable_if_t<
            ((Traits::adj_list && !Traits::fix_out_edge) || (Traits::adj_matrix && Traits::fix_out_edge)) &&
            std::is_constructible_v<node_repr_t<typename Traits::RealTraits>, std::piecewise_construct_t, std::tuple<>, std::tuple<Args&&...>>,
            std::void_t<decltype(std::declval<typename Traits::NodeContainer&>()
                .emplace_back(std::piecewise_construct, std::tuple<>{}, std::declval<std::tuple<Args&&...>>()))>
        >, Args...> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back(std::piecewise_construct, std::tuple<>{}, std::forward_as_tuple(std::forward<Args>(args)...));
                return ix;
            }
        };


        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, std::enable_if_t<
            (Traits::adj_matrix && !Traits::fix_out_edge) &&
            std::is_constructible_v<node_repr_t<typename Traits::RealTraits>, std::piecewise_construct_t, std::tuple<>, std::tuple<Args&&...>>,
            std::void_t<decltype(std::declval<typename Traits::NodeContainer&>()
                .emplace_back(std::piecewise_construct, std::tuple<>{}, std::declval<std::tuple<Args&&...>>())),
                decltype(std::declval<typename Traits::OutEdgeContainer>().resize(decltype(std::size(std::declval<typename Traits::OutEdgeContainer&>())){}))
            >
        >, Args...> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back(std::piecewise_construct, std::tuple<>{}, std::forward_as_tuple(std::forward<Args>(args)...));
                auto to = std::size(nodes);
                for (auto&& node : nodes)
                    Traits::RealTraits::out_edges(node).resize(to);
                return ix;
            }
        };

        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, std::enable_if_t<
            Traits::adj_list && Traits::fix_out_edge && !Traits::has_edge_prop &&
            std::is_constructible_v<node_repr_t<typename Traits::RealTraits>, std::piecewise_construct_t,
                std::tuple<typename Traits::OutEdgeContainer&&>, std::tuple<Args&&...>> &&
                is_aggregate_initializable_v<
                    typename Traits::OutEdgeContainer,
                    typename Traits::RealTraits::node_index_t,
                    Traits::out_edges
                >,
            std::void_t<
                decltype(std::declval<typename Traits::NodeContainer&>()
                .emplace_back(std::piecewise_construct, std::declval<std::tuple<typename Traits::OutEdgeContainer&&>>(), std::declval<std::tuple<Args&&...>>()))>
        >, Args...> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back(std::piecewise_construct,
                                   std::forward_as_tuple(aggregate_initialize<
                                       typename Traits::OutEdgeContainer
                                   >(std::make_index_sequence<Traits::out_edges>{}, Traits::RealTraits::invalid)),
                                   std::forward_as_tuple(std::forward<Args>(args)...));
                return ix;
            }
        };


        template<class Traits, class ...Args>
        struct check_emplace_impl<Traits, std::enable_if_t<
            Traits::adj_list && Traits::fix_out_edge && Traits::has_edge_prop &&
            std::is_constructible_v<node_repr_t<typename Traits::RealTraits>, std::piecewise_construct_t,
                std::tuple<typename Traits::OutEdgeContainer&&>, std::tuple<Args&&...>> &&
            is_aggregate_initializable_v<
                typename Traits::OutEdgeContainer,
                typename Traits::RealTraits::edge_repr_type,
                Traits::out_edges
            >,
            std::void_t<
                decltype(std::declval<typename Traits::NodeContainer&>()
                    .emplace_back(std::piecewise_construct, std::declval<std::tuple<typename Traits::OutEdgeContainer&&>>(), std::declval<std::tuple<Args&&...>>()))>
        >, Args...> : std::true_type {
            template<class Graph>
            constexpr static decltype(auto) do_it(Graph& g, Args&&...args) {
                auto& nodes = Traits::RealTraits::get_nodes(g);
                typename Traits::RealTraits::node_index_t ix = std::size(nodes);
                nodes.emplace_back(std::piecewise_construct,
                                   std::forward_as_tuple(aggregate_initialize<
                                       typename Traits::OutEdgeContainer
                                   >(std::make_index_sequence<Traits::out_edges>{}, typename Traits::RealTraits::edge_repr_type{Traits::RealTraits::invalid, {}})),
                                   std::forward_as_tuple(std::forward<Args>(args)...));
                return ix;
            }
        };

        template<class ...Args>
        using check_emplace = check_emplace_impl<add_node_traits, void, Args...>;
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
    /*
     *
    (1) add_node (Graph&) // !user_node_index && !has_node_prop
    (2) add_node (Graph&, Args&&...) // !user_node_index && has_node_prop
    (3) add_node (Graph&, const node_t<GraphTraits>&) // user_node_index && !has_node_prop
    (4) add_node (Graph&, const node_t<GraphTraits>&, Args&&...) // user_node_index && has_node_prop


user_node_index (==> !adj_matrix)

    .emplace(ix, {}) // (3)
     > adj_list && out_edge_size == 0 && !has_node_prop
    - `map<node_index, range<node_index>`
    - `map<node_index, range<pair<node_index, edge_prop>>`
    - `map<node_index, map<node_index, edge_prop>>`
    - `map<node_index, node_prop>`

    .emplace_back(ix, c_t_range<integer>{{-1}...}) // repeat: out_edge_size // (3)
     > adj_list && out_edge_size != 0 && !has_node_prop
    - `map<node_index, c_t_range<node_index>`


    .emplace_back(ix, c_t_range<pair<node_index, edge_prop>>{{{-1}, {edge_prop{}}...}) // repeat: out_edge_size // (3)
    - `map<node_index, c_t_range<pair<node_index, edge_prop>>`


    .emplace(inplace, ix, tuple{Args...}) // (4)
     > edge_list && has_node_prop

    .emplace(inplace, ix, tuple{inplace, {}, tuple{Args...}}) // (4)
     > adj_list && out_edge_size == 0 && has_node_prop
    - `map<node_index, pair<range<node_index>, node_prop>>`
    - `map<node_index, pair<map<node_index, edge_prop>, node_prop>>`

    .emplace(inplace, ix, tuple{inplace, {c_t_range<integer>{{-1}...}}, tuple{Args...}}) // (4)
    - `map<node_index, pair<c_t_range<node_index>, node_prop>>`

    .emplace(inplace, ix, tuple{inplace, {c_t_range<integer>{{-1, edge_prop{}}...}}, tuple{Args...}}) // (4)
    - `map<node_index, pair<c_t_map<node_index, edge_prop>, node_prop>>`

*/
    /*
    template<class Graph, class GraphTraits = graph_traits<Graph>      [, class ...Args           ]>
// parameters:                only if node index is user defined | only if it has node_property
    constexpr auto add_node(Graph&[, const node_t<GraphTraits>&    ]   [, Args&& ...              ])
    -> std::pair<node_t<GraphTraits>, bool>; // index + was new


    template<class Graph, class GraphTraits = graph_traits<Graph>>
    constexpr bool remove_node(Graph&, const node_t<GraphTraits>&);
*/
}

#endif // BXLX_GRAPH_TRAITS_GRAPH_NODE_HPP
