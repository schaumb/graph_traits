//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


struct my_range {};
namespace std {  // before include any std::begin
struct predeclared const* begin(my_range const&);
struct predeclared const* end(my_range const&);
}

#include "femto_test.hpp"
#include <bxlx/graph>
#include <array>
#include <vector>
#include <deque>
#include <list>
#include <bitset>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <forward_list>
#include <regex>
#include <atomic>
#include <cstdint>


TEST(check_bitset) {
  using namespace bxlx::graph::classification;
  struct predeclared;

  S_ASSERT(classify<std::vector<bool>> == type::bitset);
  S_ASSERT(classify<std::bitset<10>> == type::bitset);
  S_ASSERT(classify<const std::vector<bool>> == type::bitset);
  S_ASSERT(classify<const std::bitset<10>> == type::bitset);
  S_ASSERT(classify<std::bitset<0>> == type::bitset);

  struct my_bitset {
    struct bool_proxy {
        bool_proxy() = default;
        bool_proxy(bool&) = delete;
        operator bool() const noexcept;
    };

    bool_proxy operator[](size_t) const;
    size_t size() const;
  };

  S_ASSERT(classify<my_bitset> == type::bitset);
}

TEST(check_maps) {
  using namespace bxlx::graph::classification;
  struct predeclared;

  S_ASSERT(classify<std::map<int, int>> == type::map_like);
  S_ASSERT(classify<std::multimap<int, int>> == type::map_like);
  S_ASSERT(classify<std::unordered_map<int, int>> == type::map_like);
  S_ASSERT(classify<std::unordered_multimap<int, int>> == type::map_like);
  S_ASSERT(classify<std::map<predeclared, int>> == type::map_like);
  S_ASSERT(classify<std::multimap<int, std::optional<predeclared>>> == type::map_like);
  S_ASSERT(classify<std::unordered_map<int, predeclared>> == type::map_like);
  S_ASSERT(classify<std::unordered_multimap<std::pair<predeclared, predeclared>, int>> == type::map_like);

  using K = int;
  using V = int;
  struct my_map {
    struct const_iterator {
        const_iterator& operator++();
        std::pair<K, V> const& operator*() const;
        bool operator!=(const_iterator const &) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
    std::pair<const_iterator, const_iterator> equal_range(K const&) const;

    // ezek kozul az egyik:
    using key_type = K;
    V const& at(K const&) const;
    std::pair<const_iterator, const_iterator> equal_range(std::pair<K, V> const&) const = delete;
  };

  S_ASSERT(classify<my_map> == type::map_like);
}

TEST(check_ranges) {
  using namespace bxlx::graph::classification;
  struct predeclared;

  S_ASSERT(classify<bool[10]> == type::range);
  S_ASSERT(classify<const predeclared[10]> == type::range);
  S_ASSERT(classify<std::initializer_list<bool>> == type::range);
  S_ASSERT(classify<std::initializer_list<predeclared>> == type::range);
  S_ASSERT(classify<std::array<predeclared, 10>> == type::range);
  S_ASSERT(classify<std::array<int, 10>> == type::range);
  S_ASSERT(classify<std::vector<predeclared>> == type::range);
  S_ASSERT(classify<std::deque<bool>> == type::range);
  S_ASSERT(classify<std::forward_list<bool>> == type::range);
  S_ASSERT(classify<std::list<bool>> == type::range);
  S_ASSERT(classify<const std::set<predeclared>> == type::range);
  S_ASSERT(classify<std::unordered_set<predeclared>> == type::range);
  S_ASSERT(classify<std::multiset<predeclared>> == type::range);
  S_ASSERT(classify<std::unordered_multiset<predeclared>> == type::range);

#if !defined(__GNUC__) || defined(__clang__)
  // TODO gcc fails
  S_ASSERT(classify<std::match_results<const char*>> == type::range);
#endif

#if !defined(_MSC_VER) || defined(__clang__)
  // TODO msvc fails
  S_ASSERT(classify<std::vector<std::pair<predeclared, bool>>> == type::range);
  S_ASSERT(classify<std::array<std::tuple<predeclared, float>, 10>> == type::range);
#endif

  struct my_range_2 {
    struct const_iterator {
      const_iterator& operator++();
      struct predeclared const& operator*() const;
      bool operator!=(const_iterator const &) const;
    };
    const_iterator begin() const;
    const_iterator end() const;
  };
  S_ASSERT(classify<my_range> == type::range);
  S_ASSERT(classify<my_range_2> == type::range);
}

struct my_tuple {};

namespace std {
template<>
struct tuple_size<my_tuple> : integral_constant<size_t, 2> {};
template<>
struct tuple_element<0, my_tuple> { using type = struct predeclared; };
template<>
struct tuple_element<1, my_tuple> { using type = struct predeclared; };
template<size_t N>
tuple_element_t<N, my_tuple> const& get(my_tuple const&);
}

TEST(check_tuples) {
  using namespace bxlx::graph::classification;
  struct predeclared;

  S_ASSERT(classify<std::tuple<int, float>> == type::tuple_like);
  S_ASSERT(classify<const std::tuple<predeclared>> == type::tuple_like);
  S_ASSERT(classify<std::pair<const predeclared, predeclared>> == type::tuple_like);
  S_ASSERT(classify<std::pair<std::tuple<predeclared>, predeclared>> == type::tuple_like);
  S_ASSERT(classify<std::tuple<std::tuple<predeclared, const predeclared>, predeclared>> == type::tuple_like);

  S_ASSERT(classify<my_tuple> == type::tuple_like);
}

TEST(check_optionals) {
  using namespace bxlx::graph::classification;
  struct predeclared;

  S_ASSERT(classify<std::pair<predeclared, predeclared>*> == type::optional);
  S_ASSERT(classify<const char* const* const> == type::optional);
  S_ASSERT(classify<void(*)()> == type::optional);
  S_ASSERT(classify<std::optional<int>> == type::optional);
  S_ASSERT(classify<std::optional<predeclared>> == type::optional);
  S_ASSERT(classify<std::optional<std::pair<predeclared, predeclared>>> == type::optional);
  S_ASSERT(classify<std::unique_ptr<std::tuple<int, predeclared>>> == type::optional);
  S_ASSERT(classify<std::unique_ptr<predeclared>> == type::optional);
  S_ASSERT(classify<std::shared_ptr<std::optional<int>>> == type::optional);
  S_ASSERT(classify<std::shared_ptr<predeclared>> == type::optional);

  struct my_optional {
    operator bool() const;
    int const& operator*() const;
  };
  S_ASSERT(classify<my_optional> == type::optional);
}

TEST(check_bools) {
  using namespace bxlx::graph::classification;
  S_ASSERT(classify<bool> == type::bool_t);
  S_ASSERT(classify<std::vector<bool>::reference> == type::bool_t);
  S_ASSERT(classify<std::bitset<10>::reference> == type::bool_t);
  S_ASSERT(classify<std::atomic_bool> == type::bool_t);

  /*
  struct my_bool {
    my_bool(bool);
    operator bool() const;
  };
  S_ASSERT(classify<my_bool> == type::bool_t);
*/

}

TEST(check_indices) {
  using namespace bxlx::graph::classification;

  S_ASSERT(classify<size_t> == type::index);
  S_ASSERT(classify<const std::uint8_t> == type::index);
  S_ASSERT(classify<std::atomic_int> == type::index);

  /*
  struct my_index {
    my_bool(int);
    operator int() const;
  };
  S_ASSERT(classify<my_index> == type::index);
*/
}

TEST(check_indeterminates_and_predeclareds) {
  using namespace bxlx::graph::classification;
  struct predeclared;
  S_ASSERT(classify<std::reference_wrapper<bool>> == type::indeterminate);
  S_ASSERT(classify<std::reference_wrapper<int>> == type::indeterminate);
  S_ASSERT(classify<std::reference_wrapper<predeclared>> == type::indeterminate);
  S_ASSERT(classify<std::string> == type::indeterminate);
  S_ASSERT(classify<std::string_view> == type::indeterminate);
  S_ASSERT(classify<std::unique_ptr<predeclared[]>> == type::indeterminate);
#if !defined(__clang_major__) || __clang_major__ != 9
  S_ASSERT(classify<std::shared_ptr<int[]>> == type::indeterminate);
#endif
  S_ASSERT(classify<void()> == type::indeterminate);
  S_ASSERT(classify<void(predeclared::*)()> == type::indeterminate);

  S_ASSERT(classify<predeclared> == type::pre_declared);

  struct nothing {};
  S_ASSERT(classify<nothing> == type::indeterminate);
}