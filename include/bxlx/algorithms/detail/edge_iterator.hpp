#ifndef BXLX_GRAPH_EDGE_ITERATOR_HPP
#define BXLX_GRAPH_EDGE_ITERATOR_HPP

#include "../interface.hpp"
#include "bitset_iterator.hpp"
#include "getter_types.hpp"

namespace bxlx::graph::iterator {

template<class G, class Traits>
struct edge_iterable<G, Traits, std::enable_if_t<!has_node_container_v<G, Traits> && has_adjacency_container_v<G, Traits>>> {
  G& g;
  node_t<G, Traits> start;

  struct const_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<node_t<G, Traits>, /*edge_repr_t<G, Traits>*/ void*>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    iterator::bitset_iterator<adjacency_container_t<G, Traits>> it;
    node_t<G, Traits> start;

    constexpr bool operator!=(const_iterator const& rhs) const {
      return it != rhs.it;
    }

    constexpr const_iterator& operator++() {
      ++it;
      return *this;
    }

    constexpr value_type operator*() const noexcept {
      return {it.index - start, nullptr};
    }
  };

  constexpr const_iterator begin() const {
    auto c = node_count(g);
    return {iterator::get_first_good(detail::adjacent_container_getter<G, Traits>{}(g),
                                     start * c, (start+1)*c), start * c};
  }

  constexpr const_iterator end() const {
    return {};
  }

  constexpr std::size_t size() const {
    return std::distance(begin(), end());
  }
};

template<class G, class Traits>
struct edge_iterable<G, Traits, std::enable_if_t<has_node_container_v<G, Traits> && has_adjacency_container_v<G, Traits>>> {
  G& g;
  node_t<G, Traits> start;

  struct const_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<node_t<G, Traits>, void* /*, edge_repr_t<G, Traits> */>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    using wrapper_it = std::conditional_t<std::is_const_v<G>,
                                          typename adjacency_container_t<G, Traits>::const_iterator,
                                          typename adjacency_container_t<G, Traits>::iterator>;
    G* g;
    wrapper_it start, it;

    constexpr bool operator!=(const_iterator const& rhs) const {
      return it != rhs.it;
    }

    constexpr const_iterator& operator++() {
      ++it;
      return *this;
    }

    constexpr value_type operator*() const {
      /*edge_repr_t<G, Traits> edge_repr;
      if constexpr (has_edge_container_v<G, Traits>) {
        edge_repr = get_edge(g, detail::edge_index_getter<G, Traits>{}(it));
      } else {
        edge_repr = it;
      }*/
      node_t<G, Traits> node;
      if constexpr (is_user_defined_node_type_v<G, Traits> || representation_v<G, Traits> != representation_t::adjacency_matrix) {
        node = detail::composition_t<detail::first_getter_t, detail::indirect_t>{}(it);
      } else {
        node = std::distance(start, it);
      }
      return {node, nullptr};
    }
  };

  constexpr const_iterator begin() const {
    if (auto has_adj = adjacents(&g, start)) {
      return {&g, std::begin(*has_adj), std::begin(*has_adj)};
    }
    return {};
  }

  constexpr const_iterator end() const {
    if (auto has_adj = adjacents(&g, start)) {
      return {&g, std::begin(*has_adj), std::end(*has_adj)};
    }
    return {};
  }

  constexpr std::size_t size() const {
    return std::distance(begin(), end());
  }
};

template<class G, class Traits, class Check, class Neigh>
struct edge_list_iterable {
  G& g;
  node_t<G, Traits> start;

  struct const_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair<node_t<G, Traits>, void* /*edge_repr_t<G, Traits> */>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    using wrapper_it = std::conditional_t<std::is_const_v<G>,
                                          bxlx::graph::type_traits::detail::std_begin_t<const edge_list_container_t<G, Traits>>,
                                          bxlx::graph::type_traits::detail::std_begin_t<edge_list_container_t<G, Traits>>>;
    const edge_list_iterable* that;
    wrapper_it it, end;

    constexpr const_iterator(const edge_list_iterable& that, wrapper_it it, wrapper_it end) : that(&that), it(it), end(end) {
      if (it != end && Check{}(it) != that.start)
        ++*this;
    }

    constexpr bool operator!=(const_iterator const& rhs) const {
      return it != rhs.it;
    }

    constexpr const_iterator& operator++() {
      while (++it != end) {
        if (Check{}(it) == that->start)
          break;
      }
      return *this;
    }

    constexpr value_type operator*() const {
      if constexpr (has_edge_container_v<G, Traits>) {
        return {Neigh{}(it), /* get_edge(that->g, detail::edge_index_getter<G, Traits>{}(it))*/ nullptr};
      } else {
        return {Neigh{}(it), /* it*/ nullptr};
      }
    }
  };

  constexpr const_iterator begin() const {
    auto& el = detail::edge_list_container_getter<G, Traits>{}(g);
    return {*this, std::begin(el), std::end(el)};
  }

  constexpr const_iterator end() const {
    auto it = std::end(detail::edge_list_container_getter<G, Traits>{}(g));
    return {*this, it, it};
  }

  constexpr std::size_t size() const {
    return std::distance(begin(), end());
  }
};

template<class G, class Traits>
struct edge_iterable<G, Traits, std::enable_if_t<has_edge_list_container_v<G, Traits>>>
      : edge_list_iterable<G, Traits, detail::source_getter<G, Traits>, detail::target_getter<G, Traits>> {
};


template<class G, class Traits>
struct in_edge_iterable<G, Traits, std::enable_if_t<has_edge_list_container_v<G, Traits>>>
      : edge_list_iterable<G, Traits, detail::target_getter<G, Traits>, detail::source_getter<G, Traits>> {
};

}

#endif //BXLX_GRAPH_EDGE_ITERATOR_HPP
