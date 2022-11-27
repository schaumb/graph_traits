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
        struct of {
            using type = std::conditional_t<
                (Traits::container_size > 0),
                std::array<Type, Operation{}(Traits::container_size, addition)>,
                std::vector<Type>
            >;

            constexpr static type init(const Graph& g) {
                if constexpr (Traits::max_node_compile_time > 0) {
                    return {};
                } else {
                    return type(Operation{}(std::size(Traits::get_data(g)), addition));
                }
            }
        };
    };
}

#endif //BXLX_GRAPH_TRAITS_GRAPH_STORAGE_HPP
