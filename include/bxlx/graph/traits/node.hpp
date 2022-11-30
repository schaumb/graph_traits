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

/*
    template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
    constexpr auto get_node(Graph [const|&|*] g, const node_t<GraphTraits>&)
    -> node_repr_t<GraphTraits> [const|&|*];


    template<class Graph, class GraphTraits = graph_traits<std::decay_t<Graph>>>
    constexpr auto get_node_property(Graph [const|&|*] g, const node_t<GraphTraits>&)
    -> node_prop_t<GraphTraits> [const|&|*];


// only for modifiable structures:

    template<class Graph, class GraphTraits = graph_traits<Graph>      [, class ...Args           ]>
// parameters:                only if node index is user defined | only if it has node_property
    constexpr auto add_node(Graph&[, const node_t<GraphTraits>&    ]   [, Args&& ...              ])
    -> std::pair<node_t<GraphTraits>, bool>; // index + was new


    template<class Graph, class GraphTraits = graph_traits<Graph>>
    constexpr bool remove_node(Graph&, const node_t<GraphTraits>&);
*/
}

#endif // BXLX_GRAPH_TRAITS_GRAPH_NODE_HPP
