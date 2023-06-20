//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_SEARCH_HPP
#define BXLX_GRAPH_SEARCH_HPP

#include "bxlx/algorithms/detail/node_set.hpp"
#include "constants.hpp"

namespace bxlx::graph {

struct edge_types {
  enum type { tree, forward_or_cross, reverse, not_tree = forward_or_cross | reverse };
  using tree_t = std::integral_constant<type, tree>;
  using forward_or_cross_t = std::integral_constant<type, forward_or_cross>;
  using reverse_t = std::integral_constant<type, reverse>;
  using not_tree_t = std::integral_constant<type, not_tree>;
};
using edge_type = edge_types::type;

struct node_types {
  enum type { pre_visit, post_visit };
  using pre_visit_t = std::integral_constant<type, pre_visit>;
  using post_visit_t = std::integral_constant<type, post_visit>;
};
using node_type = node_types::type;

constexpr static auto with_out_edges = [] (auto& g, auto from) {
  return bxlx::graph::out_edges(g, from);
};

constexpr static auto with_in_edges = [] (auto& g, auto from) {
  return bxlx::graph::in_edges(g, from);
};

constexpr static auto with_all_edges = [] (auto& g, auto from) {
  return bxlx::graph::in_out_edges(g, from);
};

namespace detail {
  template<class ...Types>
  using tuple_t = std::tuple<Types...>;

  template<class It, class Tup, class = void>
  constexpr static bool can_assign_with_tup = false;
  template<class It, class Tup>
  constexpr static bool can_assign_with_tup<It, Tup, std::void_t<
                                                           decltype(*std::declval<It>() = std::declval<Tup>())
                                                           >> = true;

  template<class It, class ...Types>
  constexpr static bool can_assign_any = can_assign_with_tup<It, tuple_t<Types...>>;

  template<class It, class Node, class Val>
  using dfs_need_states = std::integral_constant<std::size_t, 2 + (can_assign_any<It, Node, Node, Val, edge_types::reverse_t> ||
                                                                   can_assign_any<It, Node, Node, Val, edge_types::forward_or_cross_t>)>;

  constexpr std::integral_constant<std::size_t, 0> white {};
  constexpr std::integral_constant<std::size_t, 1> grey {};
  constexpr std::integral_constant<std::size_t, 2> black {};
}

template<class Dist = size_t, class OutIt,
          class G, class Traits = graph_traits<G>,
                class NodeSetT = detail::node_set_t<G, Traits, detail::dfs_need_states<OutIt, node_t<G, Traits>, edge_repr_t<G, Traits>>>,
                auto* edges_as_neighbours = &with_out_edges>
constexpr OutIt depth_first_search(G const& g, node_t<G, Traits> from,
                                   OutIt out, NodeSetT&& nodes = {}, Dist max_dist = ~Dist()) {
  using NodeSet = std::remove_reference_t<NodeSetT>;
  constexpr auto recursive = [](auto recursive, G const& g, OutIt& out, NodeSet& nodes, node_t<G, Traits> from, Dist max_dist, Dist distance = 0) -> void {

    if constexpr (detail::can_assign_any<OutIt, node_t<G, Traits>, node_types::pre_visit_t, Dist>) {
      *out++ = detail::tuple_t<node_t<G, Traits>, node_types::pre_visit_t, Dist>{from, node_types::pre_visit_t{}, distance};
    }

    if constexpr (type_traits::range_type_v<NodeSet> == type_traits::range_type_t::set_like) {
      nodes.insert(from);
    } else {
      nodes[from] = detail::grey;
    }

    if (max_dist > distance) {
      for (auto [to, val] : (*edges_as_neighbours)(g, from)) {
        const auto current_state = [] (NodeSet& nodes, node_t<G, Traits> const& to_node) {
          if constexpr (type_traits::range_type_v<NodeSet> == type_traits::range_type_t::set_like) {
            return nodes.count(to_node);
          } else {
            return nodes[to_node];
          }
        } (nodes, to);

        switch (static_cast<std::size_t>(current_state)) {
        case detail::white:
          if constexpr (
                detail::can_assign_any<OutIt, node_t<G, Traits>, node_t<G, Traits>/*, edge_repr_t<G, Traits>*/, edge_types::tree_t>
                      ) {
            *out++ = {from, to/*, val*/, edge_types::tree_t{}};
          }
          recursive(recursive, g, out, nodes, to, max_dist, distance + 1);
          break;
        case detail::grey:
          if constexpr (
                (type_traits::range_type_v<NodeSet> != type_traits::range_type_t::set_like ||
                 type_traits::is_associative_multi_v<NodeSet>) &&
                !type_traits::is_bool_v<decltype(current_state)> &&
                detail::can_assign_any<OutIt, node_t<G, Traits>, node_t<G, Traits>/*, edge_repr_t<G, Traits>*/, edge_types::reverse_t>
                ) {
            *out++ = {from, to/*, val*/, edge_types::reverse_t{}};
          } else if constexpr (
                detail::can_assign_any<OutIt, node_t<G, Traits>, node_t<G, Traits>/*, edge_repr_t<G, Traits>*/, edge_types::not_tree_t>
                ) {
            *out++ = {from, to/*, val*/, edge_types::not_tree_t{}};
          }
          break;
        default:
          if constexpr (
                (type_traits::range_type_v<NodeSet> != type_traits::range_type_t::set_like ||
                 type_traits::is_associative_multi_v<NodeSet>) &&
                !type_traits::is_bool_v<decltype(current_state)> &&
                detail::can_assign_any<OutIt, node_t<G, Traits>, node_t<G, Traits>/*, edge_repr_t<G, Traits>*/, edge_types::forward_or_cross_t>) {
            *out++ = {from, to/*, val*/, edge_types::forward_or_cross_t{}};
          } else
          if constexpr (detail::can_assign_any<OutIt, node_t<G, Traits>, node_t<G, Traits>/*, edge_repr_t<G, Traits>*/, edge_types::not_tree_t>) {
            *out++ = {from, to/*, val*/, edge_types::not_tree_t{}};
          }
          continue;
        }
      }
    }

    if constexpr (type_traits::range_type_v<NodeSet> == type_traits::range_type_t::set_like) {
      if constexpr (type_traits::is_associative_multi_v<NodeSet>) {
        nodes.insert(from);
      }
    } else {
      nodes[from] = detail::black;
    }

    if constexpr (detail::can_assign_any<OutIt, node_t<G, Traits>, node_types::post_visit_t, Dist>) {
      *out++ = {from, node_types::post_visit_t{}, distance};
    }
  };
  recursive(recursive, g, out, nodes, from, max_dist);
  return out;
}

}

#endif //BXLX_GRAPH_SEARCH_HPP
