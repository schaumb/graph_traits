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


namespace bxlx::graph {

namespace detail {
  template<class G, class Traits = graph_traits<G>>
  constexpr bool is_strongly_connected(G const& g) {
    return false;
  }

  template<class G, class Traits = graph_traits<G>>
  constexpr bool is_weakly_connected(G const& g) {
    std::size_t count{};
    struct It {
      std::size_t& count;
      constexpr const It& operator++() const {
        return *this;
      }
      constexpr const It& operator*() const {
        return *this;
      }
      constexpr const It& operator=(std::pair<node_t<G, Traits>, edge_types::tree_t> const&) const {
        ++count;
        return *this;
      }
    } it{count};
    depth_first_search(g, *std::begin(node_indices(g)), it);

    return count == node_count(g);
  }

  template<class G, class Traits = graph_traits<G>>
  constexpr bool is_weakly_both_side_connected(G const& g) {
    node_t<G, Traits> start = *std::begin(node_indices(g));
    return false;
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
