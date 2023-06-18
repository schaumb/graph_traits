
#ifndef BXLX_GRAPH_INTERFACE_HPP
#define BXLX_GRAPH_INTERFACE_HPP

#include <type_traits>

#include "../classify/type_traits.hpp"
#include "detail/constants.hpp"
#include "../recognize/graph_traits.hpp"

namespace bxlx::graph {
namespace iterator {
  using good_bits = std::true_type;
  using bad_bits = std::false_type;
  struct all_bits { constexpr inline std::true_type operator==(bool) const { return {}; } };

  template<class T, class which_bits = good_bits,
            class Index = type_traits::detail::std_size_t<T>, bool is_const = std::is_const_v<T>>
  struct bitset_iterator;

  template<class T, class U, class WhichBits, class Index, bool is_const, bool other_const>
  constexpr std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, bool> operator != (
        bitset_iterator<T, WhichBits, Index, is_const> const& lhs,
        bitset_iterator<U, WhichBits, Index, other_const> const& rhs);


  template<class G, class Traits, class Cmp = void, class = void>
  struct node_iterable;

  template<class G, class Traits, class = void>
  struct edge_iterable;

  template<class G, class Traits, class = void>
  struct in_edge_iterable;
}
namespace detail {
  using type_traits::detail::copy_cvref_t;
  using store_bool = std::integral_constant<std::size_t, 2>;

  template<class G, class Traits, class States, class = void>
  struct node_set;

  template<class G, class Traits = graph_traits<G>, class States = store_bool, bool = it_is_a_graph_v<G, Traits>>
  using node_set_t = typename node_set<G, Traits, States>::type;
}

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto nodes(G&& graph) -> detail::copy_cvref_t<G&&, node_container_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto edges(G&& graph) -> detail::copy_cvref_t<G&&, edge_container_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto edge_list(G&& graph) -> detail::copy_cvref_t<G&&, edge_list_container_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto adjacents(G&& graph, node_t<G, Traits, true> const& node)
      -> detail::copy_cvref_t<G&&, adjacency_container_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto adjacents(G* graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G, adjacency_container_t<G, Traits>>*;

template <class G, class Traits = graph_traits<G>>
constexpr auto in_adjacents(G&& graph, node_t<G, Traits, true> const& node)
      -> detail::copy_cvref_t<G&&, in_adjacency_container_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto in_adjacents(G* graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G, in_adjacency_container_t<G, Traits>>*;

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto invalid_edge(G const&) -> edge_repr_t<G, Traits>;

template <class G, class Traits = graph_traits<G>>
constexpr auto get_edge(G& graph, edge_t<G, Traits> const& edge) -> edge_repr_t<G, Traits>;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto node_count(G const& graph)
      -> std::enable_if_t<has_node_container_v<G, Traits> || has_adjacency_container_v<G, Traits>, std::size_t>;

template<class Cmp = std::equal_to<>, class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto node_count(G const& graph, Cmp&& cmp = {})
      -> std::enable_if_t<!has_node_container_v<G, Traits> && !has_adjacency_container_v<G, Traits>, std::size_t>;

template <class G, class Traits = graph_traits<G>>
constexpr auto get_edge(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> edge_repr_t<G, Traits>;

template <class G, class Traits = graph_traits<G>>
constexpr auto equal_edges(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> std::pair<edge_repr_t<G, Traits>, edge_repr_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto equal_edges(G& graph, edge_t<G, Traits> const& edge)
      -> std::pair<edge_repr_t<G, Traits>, edge_repr_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto get_adjacency(G& graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> edge_repr_t<G, Traits>;

template <class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto graph_property(G&& graph) -> detail::copy_cvref_t<G&&, graph_property_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto node_property(G&& graph, node_t<G, Traits, true> const& node)
      -> detail::copy_cvref_t<G&&, node_property_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto node_property(G* graph, node_t<G, Traits> const& node)
      -> detail::copy_cvref_t<G, node_property_t<G, Traits>>*;

template <class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G&&, edge_repr_t<G, Traits, true> repr)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G&& graph, node_t<G, Traits, true> const& from, node_t<G, Traits, true> const& to)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G* graph, node_t<G, Traits> const& from, node_t<G, Traits> const& to)
      -> detail::copy_cvref_t<G, edge_property_t<G, Traits>>*;

template <class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G&& graph, edge_t<G, Traits, true> const& edge)
      -> detail::copy_cvref_t<G&&, edge_property_t<G, Traits>>;

template <class G, class Traits = graph_traits<G>>
constexpr auto edge_property(G* graph, edge_t<G, Traits> const& edge)
      -> detail::copy_cvref_t<G, edge_property_t<G, Traits>>*;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr std::size_t edge_count(G const& graph);

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr std::size_t adjacency_count(G const& graph);

template<class G, class Traits = graph_traits<G>>
constexpr auto has_node(G const& graph, node_t<G, Traits> const& node)
      -> std::enable_if_t<has_node_container_v<G, Traits> || has_adjacency_container_v<G, Traits>, bool>;

template<class Cmp = std::equal_to<>, class G, class Traits = graph_traits<G>>
constexpr auto has_node(G const& graph, node_t<G, Traits> const& node, Cmp&& cmp = {})
      -> std::enable_if_t<!has_node_container_v<G, Traits> && !has_adjacency_container_v<G, Traits>, bool>;

template<class G, class Traits = graph_traits<G>>
constexpr bool has_edge(G const& graph, edge_t<G, Traits> const& edge);

template<class G, class Traits = graph_traits<G>>
constexpr bool has_edge(G const& graph,
                        node_t<G, Traits> const& from,
                        node_t<G, Traits> const& to);

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr bool has_adjacency(G const& graph,
                             node_t<G, Traits> const& from,
                             node_t<G, Traits> const& to);


template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto node_indices(G& graph) -> std::enable_if_t<
      has_node_container_v<G, Traits> || has_adjacency_container_v<G, Traits>, iterator::node_iterable<G, Traits>>;

template<class Cmp = std::equal_to<>, class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto node_indices(G& graph, Cmp&& = {}) -> std::enable_if_t<
      !has_node_container_v<G, Traits> && !has_adjacency_container_v<G, Traits>, iterator::node_iterable<G, Traits, Cmp>>;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto out_edges(G& graph, node_t<G, Traits> const& node)
      -> iterator::edge_iterable<G, Traits>;

template<class G, class Traits = graph_traits<G>, bool = it_is_a_graph_v<G, Traits>>
constexpr auto in_edges(G& graph, node_t<G, Traits> const& node)
      -> iterator::in_edge_iterable<G, Traits>;
}

#endif //BXLX_GRAPH_INTERFACE_HPP
