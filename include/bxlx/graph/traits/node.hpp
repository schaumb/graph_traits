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
    constexpr static bool adj_list_v = Traits::representation == graph_representation_t::adjacency_list;
    template<class Traits>
    constexpr static bool adj_matrix_v = Traits::representation == graph_representation_t::adjacency_matrix;
    template<class Traits>
    constexpr static bool edge_list_v = Traits::representation == graph_representation_t::edge_list;


    template<class not_a_graph, class ... Args>
    constexpr std::enable_if_t<!is_it_a_graph<not_a_graph>, bool> has_node(const not_a_graph&, Args&&...) = delete;

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
        auto [from, to] = GraphTraits::get_nodes(g).equal_range(n);
        return from != to;
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
    constexpr std::enable_if_t<!is_it_a_graph<detail2::remove_cvref_t<std::remove_pointer_t<detail2::remove_cvref_t<not_a_graph>>>>, bool> get_node(not_a_graph&&, Args&&...) = delete;

    template<class Graph, class GraphTraits = graph_traits_t<Graph>>
    constexpr decltype(auto) get_node_impl(Graph& g, const node_t<GraphTraits>& n)
    {
        auto&& nodes = GraphTraits::get_nodes(g);
        if constexpr (GraphTraits::user_node_index) {
            if (auto [from, to] = nodes.equal_range(n); from != to)
                return std::addressof(*from);
        } else if (has_node(g, n)) {
            if constexpr (detail2::has_subscript_operator_v<node_container_t<GraphTraits>>) {
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
        return g ? get_node_impl(*g, n) : nullptr;
    }


    template<class not_a_graph, class ... Args>
    constexpr std::enable_if_t<!is_it_a_graph<detail2::remove_cvref_t<std::remove_pointer_t<detail2::remove_cvref_t<not_a_graph>>>>, bool>
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
        if (auto it = g ? get_node_impl(*g, n) : nullptr)
            return std::addressof(GraphTraits::get_node_property(*it));
        return nullptr;
    }

    // not a graph
    template<class not_a_graph, class ... Args>
    constexpr std::enable_if_t<!is_it_a_graph<detail2::remove_cvref_t<not_a_graph>>, std::size_t> add_node(not_a_graph&, Args&&...) = delete;

    template<bool has_node_property, class ...Args>
    constexpr static bool vararg_without_node_property = [] () -> bool {
        static_assert(has_node_property || sizeof...(Args) == 0, "Variadic arg can be passed only if node property is exists");
        return has_node_property || sizeof...(Args) == 0;
    };

    template<bool not_user_node_index, class Node, class ...Args>
    constexpr static bool missing_node_index = [] () -> bool {
        static_assert(not_user_node_index || sizeof...(Args) > 0, "Missing node index argument");
        if constexpr (sizeof...(Args) > 0) {
            static_assert(not_user_node_index || std::is_convertible_v<std::tuple_element_t<0, std::tuple<Args...>>, Node>, "Second argument is not convertible to node_index");
        }
        return not_user_node_index;
    };

    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class ...Args>
    constexpr auto add_node(Graph& g, Args&&... args)
        -> std::enable_if_t<!GraphTraits::user_node_index &&
            vararg_without_node_property<GraphTraits::has_node_property || GraphTraits::user_node_index, Args&&...> &&
            missing_node_index<!GraphTraits::user_node_index, node_t<GraphTraits>, Args...> &&
            GraphTraits::template can_add_node<Args&&...>, node_t<GraphTraits>> {
        return GraphTraits::add_node(g, std::forward<Args>(args)...);
    };

    template<class Graph, class GraphTraits = graph_traits_t<Graph>, class ...Args>
    constexpr auto add_node(Graph& g, const node_t<GraphTraits>& n, Args&&... args)
        -> std::enable_if_t<GraphTraits::user_node_index &&
            vararg_without_node_property<GraphTraits::has_node_property || !GraphTraits::user_node_index, Args&&...> &&
            GraphTraits::template can_add_node<const node_t<GraphTraits>&, Args&&...>, std::pair<node_t<GraphTraits>, bool>> {
        return GraphTraits::add_node(g, n, std::forward<Args>(args)...);
    };


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
