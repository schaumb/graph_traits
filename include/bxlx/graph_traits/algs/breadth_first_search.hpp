//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_BREADTH_FIRST_SEARCH_HPP
#define BXLX_GRAPH_TRAITS_BREADTH_FIRST_SEARCH_HPP

#include "../../graph_traits.hpp"
#include "../iterable_alg.hpp"

#include <utility>
#include <vector>
#include <tuple>
#include <array>
#include <cassert>
#include <functional>
#include <limits>

namespace bxlx::graph {
    template<class Graph, class GraphTraits = graph_traits<std::remove_const_t<std::remove_reference_t<Graph>>>>
    struct bfs_result {
        typename GraphTraits::node_index_t parent;
        typename GraphTraits::node_index_t index;
        std::size_t distance;
        typename GraphTraits::edge_repr_type* edge;
        typename GraphTraits::node_repr_type* node;
    };

    namespace detail {
        template<class Graph, class GraphTraits, graph_representation = GraphTraits::representation>
        struct bfs_impl : utils::iterable<bfs_impl<Graph, GraphTraits>, bfs_result<Graph, GraphTraits>> {
            constexpr bfs_impl(Graph&& g, typename GraphTraits::node_index_t start_index)
                : graph(std::forward<Graph>(g))
                , curr(vec.begin())
                , write(size(vec) ? std::next(curr) : curr)
            {
                if (size(vec) > start_index) {
                    curr->set_node({start_index, start_index, 0, nullptr, std::addressof(GraphTraits::get_data(graph)[start_index])});
                    vec[start_index].visited = true;
                } else if (start_index != std::numeric_limits<typename GraphTraits::node_index_t>::max() && size(vec)) {
                    throw std::overflow_error("Start node " + std::to_string(start_index) + " is bigger than size: " + std::to_string(size(vec)) + ". For an invalid element, use numeric_limits<>::max().");
                }
            }

            friend utils::iterable<bfs_impl<Graph, GraphTraits>, bfs_result<Graph, GraphTraits>>;
        private:
            constexpr void next() {
                typename GraphTraits::node_index_t neigh{};
                auto&& node = GraphTraits::get_data(graph)[curr->res.index];
                for (auto &&edge: GraphTraits::out_edges(node)) {
                    if constexpr(GraphTraits::representation == bxlx::graph_representation::adjacency_list) {
                        neigh = GraphTraits::edge_target(edge);
                    }

                    if (neigh >= size(vec)) {
                        if constexpr(GraphTraits::representation == bxlx::graph_representation::adjacency_list)
                        if (neigh == std::numeric_limits<typename GraphTraits::node_index_t>::max())
                            break;
                        throw std::overflow_error("Edge " + std::to_string(curr->res.index) + " neighbour " +
                                                  std::to_string(neigh) + " is bigger than size: " + std::to_string(size(vec)) + ". For an invalid element, use numeric_limits<>::max().");
                    }

                    if constexpr(GraphTraits::representation == bxlx::graph_representation::adjacency_matrix)
                    if (!edge) {
                        ++neigh;
                        continue;
                    }

                    if (auto &n = vec[neigh]; !n.visited) {
                        write++->set_node({curr->res.index, neigh, curr->res.distance + 1, &edge, &GraphTraits::get_data(graph)[neigh]});
                        n.visited = true;
                    }

                    if constexpr(GraphTraits::representation == bxlx::graph_representation::adjacency_matrix)
                            ++neigh;
                }
                ++curr;
            }

            constexpr bool has_end() {
                return curr == vec.end() || !curr->has_element;
            }

            constexpr auto& get_current() {
                return curr->res;
            }

            struct storage {
                bfs_result<Graph, GraphTraits> res;
                bool has_element : 1;
                bool visited : 1;

                constexpr void set_node(const bfs_result<Graph, GraphTraits>& the_res) {
                    has_element = true;
                    res = the_res;
                }
            };
            using storage_type = typename GraphTraits::template storage_t<storage>;

            Graph graph;
            storage_type vec = GraphTraits::template storage_init<storage>(graph);
            typename storage_type::iterator curr, write;
        };

        template<class Graph, class GraphTraits>
        struct bfs_impl<Graph, GraphTraits, graph_representation::adjacency_array>
            : utils::iterable<bfs_impl<Graph, GraphTraits>, bfs_result<Graph, GraphTraits>> {
            constexpr bfs_impl(Graph&& g, typename GraphTraits::node_index_t start_index)
                : graph(std::forward<Graph>(g))
                , curr(storage.begin())
                , no_info(size(storage) ? std::next(curr) : curr)
                , discarded(storage.end())
            {
                curr->parent = start_index;
                curr->index = start_index;

                std::size_t ix{};
                for (auto&& edge : GraphTraits::get_data(graph)) {
                    auto& [source, target, distance, edge_p, node_p] = storage[++ix];
                    source = GraphTraits::edge_source(edge);
                    target = GraphTraits::edge_target(edge);
                    edge_p = &edge;
                }
            }

            bool has_end() {
                return curr == no_info;
            }

            auto& get_current() {
                return *curr;
            }

            void next() {
                discarded = std::partition(std::next(curr), discarded, [t = curr->index](auto& res) {
                    return res.index != t;
                });

                auto new_no_info = std::partition(no_info, discarded, [t = curr->index](auto& res) {
                    return res.parent == t;
                });

                while (no_info != new_no_info) {
                    no_info++->distance = curr->distance + 1;
                }

                ++curr;
            }

            using storage_type = typename GraphTraits::template storage_t<bfs_result<Graph, GraphTraits>>;

            Graph graph;
            storage_type storage = GraphTraits::template storage_init<bfs_result<Graph, GraphTraits>>(graph);
            typename storage_type::iterator curr, no_info, discarded;
        };

    }

    template<class Graph, class GraphTraits = graph_traits<std::remove_const_t<std::remove_reference_t<Graph>>>>
    constexpr detail::bfs_impl<Graph, GraphTraits> bfs(Graph&& graph, typename GraphTraits::node_index_t start_index) {
        return {std::forward<Graph>(graph), start_index};
    }

    template<class Graph, class GraphTraits = graph_traits<std::remove_const_t<std::remove_reference_t<Graph>>>, class OutputIterator>
    constexpr OutputIterator bfs(Graph&& graph, typename GraphTraits::node_index_t start_index, OutputIterator out) {
        for (auto elem : bfs<Graph, GraphTraits>(std::forward<Graph>(graph), start_index))
            *out++ = elem;
        return out;
    }
}

#endif //BXLX_GRAPH_TRAITS_BREADTH_FIRST_SEARCH_HPP
