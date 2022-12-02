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

            constexpr static auto get_size(const Graph& g) {
                if constexpr (Traits::max_node_compile_time > 0) {
                    return;
                } else if constexpr (!std::is_void_v<typename Traits::node_container_type>) {
                    return Operation{}(std::size(Traits::get_nodes(g)), addition);
                } else if constexpr (Traits::max_edge_compile_time > 0) {
                    return Operation{}(Traits::max_edge_compile_time * 2, addition);
                } else {
                    return Operation{}(std::size(Traits::get_edges(g)) * 2, addition);
                }
            }

            constexpr static type init(const Graph& g) {
                if constexpr (Traits::max_node_compile_time > 0) {
                    return {};
                } else {
                    return type(get_size(g));
                }
            }

            constexpr static type reserve(const Graph& g) {
                if constexpr (Traits::max_node_compile_time > 0) {
                    return {};
                } else {
                    type res;
                    res.reserve(get_size(g));
                    return res;
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

            constexpr static auto get_size(const Graph& g) {
                if constexpr (Traits::max_edge_compile_time > 0) {
                    return;
                } else if constexpr (!std::is_void_v<typename Traits::edge_container_type>) {
                    return Operation{}(std::size(Traits::get_edges(g)), addition);
                } else if constexpr (Traits::out_edge_container_size != 0) {
                    return Operation{}(std::size(Traits::get_nodes(g)) * Traits::out_edge_container_size, addition);
                } else if constexpr (Traits::representation == traits::graph_representation::adjacency_matrix) {
                    auto&& node_size = std::size(Traits::get_nodes(g));
                    return Operation{}(node_size * node_size, addition);
                } else {
                    auto count = std::remove_reference_t<decltype(std::size(Traits::get_nodes(g)))>{};
                    for (auto&& node : Traits::get_nodes(g)) {
                        count += std::size(Traits::out_edges(node));
                    }
                    return Operation{}(count, addition);
                }
            }

            constexpr static type init(const Graph& g) {
                if constexpr (Traits::max_edge_compile_time > 0) {
                    return {};
                } else {
                    return type(get_size(g));
                }
            }

            constexpr static type reserve(const Graph& g) {
                if constexpr (Traits::max_edge_compile_time > 0) {
                    return {};
                } else {
                    type res;
                    res.reserve(get_size(g));
                    return res;
                }
            }
        };
    };
}

#endif //BXLX_GRAPH_TRAITS_GRAPH_STORAGE_HPP
