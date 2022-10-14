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
#include <execution>

namespace bxlx::graph {
    template<class Graph, class GraphTraits = graph_traits<std::remove_const_t<std::remove_reference_t<Graph>>>>
    struct breadth_first_search_result {
        typename GraphTraits::node_index_t parent;
        typename GraphTraits::node_index_t index;
        std::size_t distance;
        typename GraphTraits::edge_repr_type* edge;
        typename GraphTraits::node_repr_type* node;
    };

    namespace detail {
        template<class ExecutionPolicy>
        struct member_from_base {
            ExecutionPolicy policy;
        };

        template<class Graph, class GraphTraits, class = void, class ... ExecutionPolicy>
        struct bfs_impl : utils::iterable<bfs_impl<Graph, GraphTraits, void, ExecutionPolicy...>, breadth_first_search_result<Graph, GraphTraits>>,
                member_from_base<ExecutionPolicy> ... {
            constexpr bfs_impl(Graph&& g, typename GraphTraits::node_index_t start_index, ExecutionPolicy&&... policy)
                : member_from_base<ExecutionPolicy>{std::forward<ExecutionPolicy>(policy)}...
                , graph(std::forward<Graph>(g))
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

            friend utils::iterable<bfs_impl<Graph, GraphTraits, void, ExecutionPolicy...>, breadth_first_search_result<Graph, GraphTraits>>;
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
                breadth_first_search_result<Graph, GraphTraits> res;
                bool has_element : 1;
                bool visited : 1;

                constexpr void set_node(const breadth_first_search_result<Graph, GraphTraits>& the_res) {
                    has_element = true;
                    res = the_res;
                }
            };
            using storage_type = typename GraphTraits::template storage_t<storage>;

            Graph graph;
            storage_type vec = GraphTraits::template storage_init<storage>(graph);
            typename storage_type::iterator curr, write;
        };

        template<class Graph, class GraphTraits, class ... ExecutionPolicy>
        struct bfs_impl<Graph, GraphTraits, std::enable_if_t<GraphTraits::representation == graph_representation::adjacency_array>, ExecutionPolicy...>
            : utils::iterable<bfs_impl<Graph, GraphTraits, void, ExecutionPolicy...>, breadth_first_search_result<Graph, GraphTraits>>,
                member_from_base<ExecutionPolicy> ... {
            constexpr bfs_impl(Graph&& g, typename GraphTraits::node_index_t start_index, ExecutionPolicy&&... policy)
                : member_from_base<ExecutionPolicy>{std::forward<ExecutionPolicy>(policy)}...
                , graph(std::forward<Graph>(g))
                , curr(storage.begin())
                , no_info(size(storage) ? std::next(curr) : curr)
                , discarded(storage.end())
            {
                curr->parent = start_index;
                curr->index = start_index;

                auto&& data = GraphTraits::get_data(graph);
                std::transform(this->member_from_base<ExecutionPolicy>::policy..., begin(data), end(data), no_info, [] (auto&& edge) {
                    return breadth_first_search_result<Graph, GraphTraits>{
                        GraphTraits::edge_source(edge),
                        GraphTraits::edge_target(edge),
                        0,
                        &edge,
                        nullptr
                    };
                });
            }

            bool has_end() {
                return curr == no_info;
            }

            auto& get_current() {
                return *curr;
            }

            void next() {
                discarded = std::partition(this->member_from_base<ExecutionPolicy>::policy..., std::next(curr), discarded, [t = curr->index](auto& res) {
                    return res.index != t;
                });

                auto new_no_info = std::partition(this->member_from_base<ExecutionPolicy>::policy..., no_info, discarded, [t = curr->index](auto& res) {
                    return res.parent == t;
                });

                while (no_info != new_no_info) {
                    no_info++->distance = curr->distance + 1;
                }

                ++curr;
            }

            using storage_type = typename GraphTraits::template storage_t<breadth_first_search_result<Graph, GraphTraits>, '+', 1>;

            Graph graph;
            storage_type storage = GraphTraits::template storage_init<breadth_first_search_result<Graph, GraphTraits>, '+', 1>(graph);

            typename storage_type::iterator curr, no_info, discarded;
        };
    }


    template<class graph_t, class = void>
    struct graph_traits_traits;

    template<class graph_t>
    struct graph_traits_traits<graph_t, std::enable_if_t<!std::is_execution_policy_v<bxlx::detail::remove_cvref_t<graph_t>>>> {
        using type = bxlx::graph_traits<graph_t>;
    };

    template<class Graph, class GraphTraits = typename graph_traits_traits<Graph>::type>
    constexpr detail::bfs_impl<Graph, GraphTraits> breadth_first_search(Graph&& graph, typename GraphTraits::node_index_t start_index) {
        return {std::forward<Graph>(graph), start_index};
    }

    template<class ExecutionPolicy, class Graph, class GraphTraits =
        typename std::enable_if_t<std::is_execution_policy_v<bxlx::detail::remove_cvref_t<ExecutionPolicy>>, graph_traits_traits<Graph>>::type>
    constexpr detail::bfs_impl<Graph, GraphTraits, void, ExecutionPolicy> breadth_first_search(
        ExecutionPolicy&& policy, Graph&& graph, typename GraphTraits::node_index_t start_index) {
        return {std::forward<Graph>(graph), start_index, std::forward<ExecutionPolicy>(policy)};
    }

    template<class Graph, class GraphTraits = typename graph_traits_traits<Graph>::type, class OutputIterator>
    constexpr OutputIterator breadth_first_search(Graph&& graph, typename GraphTraits::node_index_t start_index, OutputIterator out) {
        for (auto elem : breadth_first_search<Graph, GraphTraits>(std::forward<Graph>(graph), start_index))
            *out++ = elem;
        return out;
    }

    template<class ExecutionPolicy, class Graph, class GraphTraits =
        typename std::enable_if_t<std::is_execution_policy_v<bxlx::detail::remove_cvref_t<ExecutionPolicy>>, graph_traits_traits<Graph>>::type, class OutputIterator>
    constexpr OutputIterator breadth_first_search(ExecutionPolicy&& policy, Graph&& graph, typename GraphTraits::node_index_t start_index, OutputIterator out) {
        auto&& cont = breadth_first_search<ExecutionPolicy&, Graph, GraphTraits>(policy, std::forward<Graph>(graph), start_index);
        return std::copy(std::forward<ExecutionPolicy>(policy), begin(cont), end(cont), out);
    }
}

#endif //BXLX_GRAPH_TRAITS_BREADTH_FIRST_SEARCH_HPP
