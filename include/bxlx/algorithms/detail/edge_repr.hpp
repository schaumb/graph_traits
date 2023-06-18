#ifndef BXLX_GRAPH_EDGE_REPR_HPP
#define BXLX_GRAPH_EDGE_REPR_HPP

#include "../interface.hpp"
#include "bitset_iterator.hpp"

namespace bxlx::graph::detail {

template<class G, class Traits>
struct edge_repr<G, Traits, std::enable_if_t<has_edge_container_v<G, Traits, true>>> {
  using type = std::conditional_t<std::is_const_v<edge_container_t<G, Traits, true>>,
                                  typename edge_container_t<G, Traits, true>::const_iterator,
                                  typename edge_container_t<G, Traits, true>::iterator>;
};
template<class G, class Traits>
struct edge_repr<G, Traits, std::enable_if_t<!has_edge_container_v<G, Traits> && has_edge_list_container_v<G, Traits>>> {
  using type = std::conditional_t<std::is_const_v<edge_list_container_t<G, Traits>>,
                                  bxlx::graph::type_traits::detail::std_begin_t<const edge_list_container_t<G, Traits>>,
                                  bxlx::graph::type_traits::detail::std_begin_t<edge_list_container_t<G, Traits>>>;
};
template<class G, class Traits>
struct edge_repr<G, Traits, std::enable_if_t<!has_edge_container_v<G, Traits, true> && has_adjacency_container_v<G, Traits, true> &&
                                             (classification::classify<adjacency_container_t<G, Traits, true>> != classification::type::bitset)>> {
  using type = std::conditional_t<std::is_const_v<adjacency_container_t<G, Traits, true>>,
                                  typename adjacency_container_t<G, Traits, true>::const_iterator,
                                  typename adjacency_container_t<G, Traits, true>::iterator>;
};

template<class G, class Traits>
struct edge_repr<G, Traits, std::enable_if_t<!has_edge_container_v<G, Traits, true> && has_adjacency_container_v<G, Traits, true> &&
                                             (classification::classify<adjacency_container_t<G, Traits, true>> == classification::type::bitset)>> {
  using type = iterator::bitset_iterator<adjacency_container_t<G, Traits, true>>;
};

}

#endif //BXLX_GRAPH_EDGE_REPR_HPP
