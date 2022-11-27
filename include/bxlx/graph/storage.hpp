//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_GRAPH_STORAGE_HPP
#define BXLX_GRAPH_TRAITS_GRAPH_STORAGE_HPP

#include "traits.hpp"

namespace bxlx {
    template<class Graph, class Traits = graph_traits<Graph>>
    struct storage {
        template<class Type, auto addition = 0, class Operation = std::plus<>>
        struct node {
            using type = std::conditional_t<
                (Traits::max_node_compile_time > 0),
                std::array<Type, Operation{}(Traits::max_node_compile_time, addition)>,
                std::vector<Type>
            >;

            constexpr static type init(const Graph& g) {
                if constexpr (Traits::max_node_compile_time > 0) {
                    return {};
                } else if constexpr (!std::is_void_v<std::invoke_result_t<decltype(Traits::get_nodes), Graph>>) {
                    return type(Operation{}(std::size(Traits::get_nodes(g)), addition));
                } else {
                    return type(Operation{}(std::size(Traits::get_edges(g)) * 2, addition));
                }
            }
        };
        template<class Type, auto addition = 0, class Operation = std::plus<>>
        struct edge {
            using type = std::conditional_t<
                (Traits::max_edge_compile_time > 0),
                std::array<Type, Operation{}(Traits::max_edge_compile_time, addition)>,
                std::vector<Type>
            >;

            constexpr static type init(const Graph& g) {
                if constexpr (Traits::max_edge_compile_time > 0) {
                    return {};
                } else if constexpr (!std::is_void_v<std::invoke_result_t<decltype(Traits::get_edges), Graph>>) {
                    return type(Operation{}(std::size(Traits::get_edges(g)), addition));
                } else {
                    auto&& node_size = std::size(Traits::get_nodes(g));
                    return type(Operation{}(node_size * (Traits::in_container_size ? Traits::in_container_size : node_size), addition));
                }
            }
        };
    };
}

#endif //BXLX_GRAPH_TRAITS_GRAPH_STORAGE_HPP
