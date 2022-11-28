//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_GRAPH_TRAITS_HPP
#define BXLX_GRAPH_TRAITS_GRAPH_TRAITS_HPP

#include <cstddef>
#include "traits/graph_traits.hpp"

namespace bxlx {
    using traits::graph_representation;
    using traits::is_graph_v; // sfinae friendly check.
    using traits::is_it_a_graph; // static_asserts on failure, and prints the error

    template<class graph_t>
    struct graph_traits : traits::graph_traits<graph_t> {
        using impl = traits::graph_traits<graph_t>;

        constexpr static graph_representation representation = impl::representation;
        using node_index_t = typename impl::node_index_t;

        [[maybe_unused]] constexpr static bool has_graph_property = impl::has_graph_property;
        [[maybe_unused]] constexpr static bool has_edge_property = impl::has_edge_property;
        [[maybe_unused]] constexpr static bool has_node_property = impl::has_node_property;

        // void* if not exists
        using node_repr_type = typename impl::node_repr_type;
        using edge_repr_type = typename impl::edge_repr_type;

        // void if not exists
        using graph_property_type [[maybe_unused]] = typename impl::graph_property_type;
        using edge_property_type  [[maybe_unused]] = typename impl::edge_property_type;
        using node_property_type  [[maybe_unused]] = typename impl::node_property_type;

        // 0 if it cannot be calculated
        constexpr static std::size_t max_node_compile_time = impl::max_node_compile_time;
        constexpr static std::size_t max_edge_compile_time = impl::max_edge_compile_time;
    };
}

#endif //BXLX_GRAPH_TRAITS_GRAPH_TRAITS_HPP
