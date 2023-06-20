#ifndef BXLX_GRAPH_NODE_ITERATOR_HPP
#define BXLX_GRAPH_NODE_ITERATOR_HPP

#include "getter_types.hpp"

namespace bxlx::graph::iterator {

template<class G, class Traits>
struct const_num_iterator {
  using iterator_category = std::forward_iterator_tag;
  using value_type = node_t<G, Traits>;
  using difference_type = std::ptrdiff_t;
  using pointer = const value_type*;
  using reference = const value_type&;
  value_type n;

  template<class Any>
  constexpr const_num_iterator(const Any& notG, bool end) :
        n(end ? std::size(notG) : 0) {}

  constexpr explicit const_num_iterator(std::size_t start) :
        n(start) {}

  constexpr bool operator!=(const_num_iterator const& rhs) const {
    return n != rhs.n;
  }

  constexpr const_num_iterator& operator++() {
    ++n;
    return *this;
  }

  constexpr value_type operator*() const {
    return n;
  }
};


template<class G, class Traits>
struct node_iterable<G, Traits, void, std::enable_if_t<has_node_container_v<G, Traits>>> {
  G& g;
  template<class G2 = G, class Traits2 = Traits, class = void>
  struct const_iterator : const_num_iterator<G2, Traits2> {
    using const_num_iterator<G2, Traits2>::const_num_iterator;
  };

  template<class G2, class Traits2>
  struct const_iterator<G2, Traits2, std::enable_if_t<detail::is_associative<node_container_t, G2, Traits2>>> {
    using iterator_category = std::forward_iterator_tag;
    using value_type = node_t<G, Traits>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;
    using It = typename node_container_t<G, Traits>::const_iterator;
    It it;

    constexpr const_iterator(const node_container_t<G2, Traits2>& nodes, bool end) :
          it (end ? std::end(nodes) : std::begin(nodes)){}

    constexpr bool operator!=(const_iterator const& rhs) const {
      return it != rhs.it;
    }

    constexpr const_iterator& operator++() {
      ++it;
      return *this;
    }

    constexpr value_type operator*() const {
      return detail::composition_t<detail::first_getter_t, detail::indirect_t>{}(it);
    }
  };

  constexpr const_iterator<> begin() const {
    return const_iterator<>(detail::node_container_getter<G, Traits>{}(g), false);
  }

  constexpr const_iterator<> end() const {
    return const_iterator<>(detail::node_container_getter<G, Traits>{}(g), true);
  }

  constexpr std::size_t size() const {
    return std::size(detail::node_container_getter<G, Traits>{}(g));
  }
};


template<class G, class Traits>
struct node_iterable<G, Traits, void, std::enable_if_t<!has_node_container_v<G, Traits> &&
                                                       has_adjacency_container_v<G, Traits>>> {
  G& g;

  using const_iterator = const_num_iterator<G, Traits>;

  constexpr const_iterator begin() const {
    return const_iterator(0);
  }

  constexpr const_iterator end() const {
    return const_iterator(size());
  }

  constexpr std::size_t size() const {
    return node_count(g);
  }
};


template<class G, class Traits, class Cmp>
struct node_iterable<G, Traits, Cmp, std::enable_if_t<!has_node_container_v<G, Traits> &&
                                                      !has_adjacency_container_v<G, Traits>>> {
  G& g;
  Cmp&& cmp;

  struct const_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = node_t<G, Traits>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;
    using It = bxlx::graph::type_traits::detail::std_begin_t<const edge_list_container_t<G, Traits>>;

    constexpr static auto source = detail::source_getter<G, Traits>{};
    constexpr static auto target = detail::target_getter<G, Traits>{};

    It it;
    bool first = true;
    It end;
    Cmp cmp;

    constexpr const_iterator(const edge_list_container_t<G, Traits>& list, bool end,
                             Cmp cmp) :
          it(end ? std::end(list) : std::begin(list)),
          end(std::end(list)), cmp(cmp) {
      while (it != this->end && !check()) {
        first = !first;
        if (first) {
          ++it;
        }
      }
    }

    constexpr bool check() const {
      auto v = **this;

      if (first && cmp(v, target(it)))
        return false;

      for (auto i = std::next(it); i != end; ++i) {
        if (cmp(v, source(i)) || cmp(v, target(i)))
          return false;
      }
      return true;
    }

    constexpr bool operator!=(const_iterator const& rhs) const {
      return it != rhs.it;
    }

    constexpr const_iterator& operator++() {
      while (it != end) {
        first = !first;
        if (first) {
          ++it;
        }
        if (it != end && check())
          break;
      }
      return *this;
    }

    constexpr value_type operator*() const {
      if (first) {
        return source(it);
      } else {
        return target(it);
      }
    }
  };


  constexpr const_iterator begin() const {
    return const_iterator(edge_list(g), false, cmp);
  }

  constexpr const_iterator end() const {
    return const_iterator(edge_list(g), true, cmp);
  }

  constexpr std::size_t size() const {
    return std::distance(begin(), end());
  }
};

}

#endif //BXLX_GRAPH_NODE_ITERATOR_HPP
