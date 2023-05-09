//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_DECISIONS_HPP
#define BXLX_GRAPH_DECISIONS_HPP

#include "getters.hpp"

namespace bxlx::graph {

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool is_connected(G const&) {
  return false;
}

template<class Strongly = std::nullptr_t, class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool is_connected(G const&, Strongly&& = {}) {
  return false;
}
}

#endif //BXLX_GRAPH_DECISIONS_HPP
