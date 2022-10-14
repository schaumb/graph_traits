//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_GRAPH_TRAITS_HPP
#define BXLX_GRAPH_TRAITS_GRAPH_TRAITS_HPP

#include <cstddef>
#include "graph_traits/graph_traits_impl.hpp"

namespace bxlx {
    using detail::graph_representation;

    template<class graph_t>
    struct graph_traits : detail::graph_traits<graph_t> {
        using impl = detail::graph_traits<graph_t>;

        constexpr static graph_representation representation = impl::representation;

        constexpr static bool has_graph_property = impl::has_graph_property;
        constexpr static bool has_edge_property = impl::has_edge_property;
        constexpr static bool has_node_property = impl::has_node_property;

        // void if not exists
        using graph_property_type = typename impl::graph_property_type;
        using edge_property_type = typename impl::edge_property_type;
        using node_property_type = typename impl::node_property_type;

        // void* if not exists
        using graph_repr_type = typename impl::graph_repr_type;
        using node_repr_type = typename impl::node_repr_type;
        using edge_repr_type = typename impl::edge_repr_type;

        // 0 if cannot be calculated
        constexpr static std::size_t max_node_compile_time =
            representation == graph_representation::adjacency_array ?
            impl::storage_size * 2 : impl::storage_size;

        constexpr static std::size_t max_edge_compile_time =
            representation == graph_representation::adjacency_array ?
            impl::storage_size : impl::inside_storage_size * impl::storage_size;
    };
}

#endif //BXLX_GRAPH_TRAITS_GRAPH_TRAITS_HPP
