//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_HPP
#define BXLX_GRAPH_TRAITS_HPP

#include "recognize.hpp"

namespace bxlx::graph::traits {
enum class representation_t {
  adjacency_list = 1,
  adjacency_matrix,
  edge_list,
};

namespace detail {
  struct noop_t {
    template<class ...Ts>
    constexpr void operator()(Ts && ...) const noexcept {
    }
  };

  template<std::size_t I>
  struct getter_t {
    template<class T, class ...Ts>
    [[nodiscard]] constexpr auto operator()(T&& val, Ts&&...) const noexcept -> type_traits::detail::copy_cvref_t<T&&, std::tuple_element_t<I, T>> {
      return std::get<I>(val);
    }
  };

  struct indirect_t {
    template<class T, class ...Ts>
    [[nodiscard]] constexpr auto operator()(T&& val, Ts&&...) const noexcept -> decltype(*val) {
      return *val;
    }
  };

  struct identity_t {
    template<class T, class ...Ts>
    [[nodiscard]] constexpr T&& operator()(T&& val, Ts&&...) const noexcept {
      return std::forward<T>(val);
    }
  };

  template<class type1, class type2>
  struct composition_t {
    template<class T, class ...Ts>
    [[nodiscard]] constexpr auto operator()(T&& val, Ts&&... ts) const noexcept -> std::invoke_result_t<type1, std::invoke_result_t<type2, T&&, Ts&&...>, Ts&&...> {
      return type1{}(type2{}(std::forward<T>(val), ts...), ts...);
    }
  };
}

template <class G, class V = decltype(state_machine::graph::template valid<std::remove_reference_t<G>>()), bool = V{}.value>
struct graph_traits {
  using reason_t = V;
};

template <class G, class V>
struct graph_traits<G, V, true> {
  using properties_t = typename V::properties;

  constexpr static representation_t representation =
        properties::has_property_v<properties_t, state_machine::adj_list>
              ? representation_t::adjacency_list :
        properties::has_property_v<properties_t, state_machine::adj_mat>
              ? representation_t::adjacency_matrix :
        properties::has_property_v<properties_t, state_machine::edge_list>
              ? representation_t::edge_list :
        representation_t{};

  static_assert(representation != representation_t{});

  using graph_property = properties::get_value<properties_t, state_machine::graph_prop_t>;
  using edge_property = properties::get_value<properties_t, state_machine::edge_prop_t>;
  using node_property = properties::get_value<properties_t, state_machine::node_prop_t>;

  using user_defined_node_t = properties::get_value_t<properties_t, state_machine::user_node_t, std::enable_if<true, std::false_type>>;
  using user_defined_edge_t = properties::get_value_t<properties_t, state_machine::user_edge_t>;

  using node_type = properties::get_value<properties_t, state_machine::node_type_t, std::enable_if<true, std::size_t>>;
  using edge_type =
        properties::get_value<properties_t, state_machine::edge_type_t,
        properties::get_value<properties_t, state_machine::node_pair<next_type::Dummy>>>;

  using node_container =
        properties::get_value<properties_t, state_machine::node_container<next_type::Dummy>,
        properties::get_value<properties_t, state_machine::adj_list,
                                                    std::conditional_t<properties::is_valid_v<properties_t, state_machine::compressed_t, std::false_type>,
                                                                       properties::get_value<properties_t, state_machine::adj_mat>,
                                                                       properties::undefined>
        >>;

  using edge_container = properties::get_value<properties_t, state_machine::edge_container<next_type::Dummy>>;

  using edge_list_container = properties::get_value<properties_t, state_machine::edge_list_n<next_type::Dummy>>;

  using adjacency_container =
        properties::get_value<properties_t, state_machine::indexed_type<state_machine::adj_list_cont<next_type::Dummy>, 1>,
        properties::get_value<properties_t, state_machine::indexed_type<state_machine::adj_mat_cont<next_type::Dummy>, 1>,
        properties::get_value<properties_t, state_machine::adj_list_cont<next_type::Dummy>,
        properties::get_value<properties_t, state_machine::adj_mat_cont<next_type::Dummy>>>>>;

  using in_adjacency_container = std::conditional_t<
        properties::has_property_v<properties_t, state_machine::indexed_type<state_machine::adj_list_cont<next_type::Dummy>, 1>> ||
        properties::has_property_v<properties_t, state_machine::indexed_type<state_machine::adj_mat_cont<next_type::Dummy>, 1>>,
        properties::get_value<properties_t, state_machine::adj_list_cont<next_type::Dummy>,
                              properties::get_value<properties_t, state_machine::adj_mat_cont<next_type::Dummy>>>,
        std::enable_if<false>
        >;

  using in_edges_t = properties::get_value_t<properties_t, state_machine::in_edges_t, std::enable_if<true, std::false_type>>;

  using multi_edge = properties::get_value<properties_t, state_machine::multi_edge_t>;
  using compressed_edges = properties::get_value<properties_t, state_machine::compressed_t>;
};

#undef BXLX_GRAPH_TRAITS_SIMPLIFY_GETTER
} // namespace bxlx::graph::traits

#endif //BXLX_GRAPH_TRAITS_HPP
