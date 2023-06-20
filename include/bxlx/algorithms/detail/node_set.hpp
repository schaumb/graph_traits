#ifndef BXLX_GRAPH_NODE_SET_HPP
#define BXLX_GRAPH_NODE_SET_HPP

#include "../interface.hpp"
#include "bitset_iterator.hpp"

#include <climits>
#include <set>

namespace bxlx::graph::detail {
template<class T>
constexpr T log2(T x) {
  return x == T{1} ? T{} : 1+log2(x >> 1);
}

template<class T>
constexpr T log2_ceil(T x) {
  return x == T{1} ? T{} : log2(x - 1) + 1;
}

/*
template<class G, class Traits, class States>
struct node_set<G, Traits, States,
                std::enable_if_t<max_node_size_v<G, Traits> != std::numeric_limits<std::size_t>::max()>> {
  constexpr static std::size_t BITS_PER_SET = sizeof(std::uint64_t) * CHAR_BIT;
  constexpr static std::size_t USED_BITS_PER_NODE = log2_ceil(States::value);
  constexpr static std::size_t BIT_MASK = (1 << USED_BITS_PER_NODE) - 1;
  std::array<std::uint64_t, (max_node_size_v<G, Traits> * USED_BITS_PER_NODE + (BITS_PER_SET - 1) ) / BITS_PER_SET> bitset {};

  using const_iterator = iterator::bitset_iterator<const node_set, iterator::good_bits, node_t<G, Traits>>;
  using iterator = iterator::bitset_iterator<node_set, iterator::all_bits, node_t<G, Traits>>;

  constexpr const_iterator begin() const {
    return iterator::get_first_good(*this);
  }

  constexpr const_iterator end() const {
    return {this, size(), size()};
  }

  struct reference {
    using ToType = std::conditional_t<USED_BITS_PER_NODE == 1, bool, std::size_t>;
    iterator it;

    constexpr operator ToType() const {
      auto ix = it.index;
      return (it.obj->bitset[ix / (BITS_PER_SET / USED_BITS_PER_NODE)] >> (ix % (BITS_PER_SET / USED_BITS_PER_NODE))) & BIT_MASK;
    }

    constexpr reference& operator=(std::conditional_t<USED_BITS_PER_NODE == 1, bool, std::size_t> res) {
      auto ix = it.index;
      it.obj->bitset[ix / (BITS_PER_SET / USED_BITS_PER_NODE)] ^= ((res & BIT_MASK) ^ static_cast<ToType>(*this)) << (ix % (BITS_PER_SET / USED_BITS_PER_NODE));
      return *this;
    }
  };

  constexpr reference operator[](node_t<G, Traits> const& n) {
    return {{this, n, static_cast<node_t<G, Traits>>(size())}};
  }

  constexpr std::size_t size() const noexcept {
    return max_node_size_v<G, Traits>;
  }

  using type = node_set<G, Traits, States>;
};
 */

template<class G, class Traits, class States>
struct node_set<G, Traits, States, std::enable_if_t</*max_node_size_v<G, Traits> == std::numeric_limits<std::size_t>::max() &&*/ States{} == 2>> {
  using type = std::set<node_t<G, Traits>>;
};

template<class G, class Traits, class States>
struct node_set<G, Traits, States, std::enable_if_t<(States{} > 2)>> {
  using type = std::multiset<node_t<G, Traits>>;
};
}

#endif //BXLX_GRAPH_NODE_SET_HPP
