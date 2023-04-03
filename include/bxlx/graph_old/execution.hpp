//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_GRAPH_TRAITS_IMPL_HPP
#define BXLX_GRAPH_TRAITS_GRAPH_TRAITS_IMPL_HPP

#include <execution>

namespace bxlx::graph::execution {
    [[maybe_unused]] class lazy_policy {
    } constexpr lazy{};

    template <class>
    struct is_graph_execution_policy : std::false_type
    {
    };


    template <>
    struct is_graph_execution_policy<lazy_policy> : std::true_type
    {
    };

    template <class T>
    [[maybe_unused]] constexpr bool is_graph_execution_policy_v = is_graph_execution_policy<T>::value;
}

#endif //BXLX_GRAPH_TRAITS_GRAPH_TRAITS_IMPL_HPP
