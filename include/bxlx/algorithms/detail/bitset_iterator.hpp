//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_BITSET_ITERATOR_HPP
#define BXLX_GRAPH_BITSET_ITERATOR_HPP

#include "../interface.hpp"

namespace bxlx::graph::iterator {
  template<class T, class which_bits,
            class Index, bool is_const>
  struct bitset_iterator {
    using iterator_category = std::forward_iterator_tag;
    using value_type = bool;
    using difference_type = std::ptrdiff_t;
    using pointer = const bool*;
    using reference = const bool&;


    T* obj;
    Index index;
    Index max;

    bool get_bool() const {
      return obj && max > index && **this;
    }

    constexpr bool operator*() const {
      return which_bits{} == (*obj)[index];
    }

    constexpr operator bitset_iterator<const T, which_bits, Index, true>() const {
      return {obj, index, max};
    }

    constexpr bitset_iterator& operator++ () {
      if (get_bool())
        do {
          ++index;
        } while (max > index && !**this);
      return *this;
    }
  };

  template<class T>
  constexpr bitset_iterator<T> get_first_good(T& bitset, std::size_t from = 0, std::size_t to = ~std::size_t{}) {
    bitset_iterator<T> r {&bitset, from, std::min(to, std::size(bitset))};
    while (r.index < r.max && !bitset[r.index]) {
      ++r.index;
    }
    return r;
  }

  template<class T, class U, class WhichBits, class Index, bool is_const, bool other_const>
  constexpr std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, bool> operator != (
        bitset_iterator<T, WhichBits, Index, is_const> const& lhs,
        bitset_iterator<U, WhichBits, Index, other_const> const& rhs) {
    return (lhs.get_bool() || rhs.get_bool()) && (lhs.obj != rhs.obj || lhs.index != rhs.index);
  }
}

#endif //BXLX_GRAPH_BITSET_ITERATOR_HPP
