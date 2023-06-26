//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_DECISIONS_HPP
#define BXLX_GRAPH_DECISIONS_HPP

#include "bxlx/algorithms/detail/getters.hpp"
#include "bxlx/algorithms/search.hpp"
#include "detail/edge_repr.hpp"

#include <map>

namespace bxlx::graph {

namespace detail {
  template<class G, class Traits = graph_traits<G>>
  constexpr bool is_strongly_connected(G const& g) {
    struct Node {
      std::size_t index;
      std::size_t lowlink;
    };
    std::size_t curr_index{};
    std::map<node_t<G, Traits>, Node> node_map;

    auto strong_connect = [&] (auto strong_connect, node_t<G, Traits> node) -> Node* {
      Node& e = node_map.try_emplace(node, Node{curr_index, curr_index}).first->second;
      ++curr_index;
      for (auto [to, val] : out_edges(g, node)) {
        if (auto it = node_map.find(to); it == node_map.end()) {
          auto* n = strong_connect(strong_connect, to);
          if (n == nullptr)
            return nullptr;
          e.lowlink = std::min(e.lowlink, n->lowlink);
        }
        else {
          e.lowlink = std::min(e.lowlink, it->second.index);
        }
      }
      return e.index == e.lowlink && e.index != 0 ? nullptr : &e;
    };

    return strong_connect(strong_connect, *std::begin(node_indices(g))) &&
           curr_index == node_count(g);
  }

  template<class G, class Traits = graph_traits<G>>
  constexpr bool is_weakly_connected(G const& g) {
    std::size_t count{};
    struct It {
      std::size_t& count;
      constexpr const It& operator++() const {
        return *this;
      }
      constexpr const It& operator++(int) const {
        return *this;
      }
      constexpr const It& operator*() const {
        return *this;
      }
      constexpr const It& operator=(std::tuple<node_t<G, Traits>, node_types::pre_visit_t, size_t> const&) const {
        ++count;
        return *this;
      }
    } it{count};
    depth_first_search(g, *std::begin(node_indices(g)), it);

    return count == node_count(g);
  }

  template<class G, class Traits = graph_traits<G>>
  constexpr bool is_weakly_both_side_connected(G const& g) {

    std::size_t count{};
    struct It {
      std::size_t& count;
      constexpr const It& operator++() const {
        return *this;
      }
      constexpr const It& operator++(int) const {
        return *this;
      }
      constexpr const It& operator*() const {
        return *this;
      }
      constexpr const It& operator=(std::tuple<node_t<G, Traits>, node_types::pre_visit_t, size_t> const&) const {
        ++count;
        return *this;
      }
    } it{count};
    depth_first_search<size_t, It, G, Traits, std::set<node_t<G, Traits>>, &with_all_edges>
          (g, *std::begin(node_indices(g)), it);

    return count == node_count(g);
  }

  template<class G, class Traits = graph_traits<G>>
  constexpr bool all_edge_has_both_direction(G const& g) {
    for (node_t<G, Traits> n : node_indices(g)) {
      for (auto [to, repr] : out_edges(g, n)) {
        if (!has_edge(g, to, n))
          return false;
      }
    }
    return true;
  }
}

template<class G, class Traits = graph_traits<G>>
constexpr std::enable_if_t<detail::has_directed_edges_v<G, Traits>, bool> is_connected(G const& g) {
  if constexpr (directed_edges_v<G, Traits>) {
    return detail::is_strongly_connected<G, Traits>(g);
  } else {
    return detail::is_weakly_connected<G, Traits>(g);
  }
}

template<class G, class Traits = graph_traits<G>, class Strongly = std::enable_if_t<!detail::has_directed_edges_v<G, Traits>, std::nullptr_t>>
constexpr bool is_connected(G const& g, Strongly&& s = {}) {
  if constexpr (detail::has_directed_edges_v<const G, Traits>) {
    static_assert(std::is_convertible_v<Strongly, bool>);
    if (s) {
      return is_connected<G, Traits>(g);
    } else {
      return detail::is_weakly_both_side_connected<const G, Traits>(g);
    }
  } else if constexpr (std::is_convertible_v<Strongly, bool>) {
    if (s) {
      return detail::is_strongly_connected<const G, Traits>(g);
    } else {
      if (detail::all_edge_has_both_direction<const G, Traits>(g)) {
        return detail::is_weakly_connected<const G, Traits>(g);
      } else {
        return detail::is_weakly_both_side_connected<const G, Traits>(g);
      }
    }
  } else if (detail::all_edge_has_both_direction<G, Traits>(g)){
    return detail::is_weakly_connected<G, Traits>(g);
  } else {
    return detail::is_strongly_connected<G, Traits>(g);
  }
}
}

#endif //BXLX_GRAPH_DECISIONS_HPP
