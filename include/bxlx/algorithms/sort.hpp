//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_SORT_HPP
#define BXLX_GRAPH_SORT_HPP

namespace bxlx::graph {

template<class OutIt, class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr OutIt topological_sort(G const&, OutIt out) {
  return out;
}
}

#endif //BXLX_GRAPH_SORT_HPP
