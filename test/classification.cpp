//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "femto_test.hpp"
#include <bxlx/graph>
#include <array>
#include <vector>
#include <deque>
#include <list>
#include <bitset>

TEST(check_bitset) {
  using namespace bxlx::graph::classification;
  struct predeclared;

  S_ASSERT(classify<std::vector<bool>> == type::bitset);
  S_ASSERT(classify<std::bitset<10>> == type::bitset);
  S_ASSERT(classify<const std::vector<bool>> == type::bitset);
  S_ASSERT(classify<const std::bitset<10>> == type::bitset);
  S_ASSERT(classify<std::bitset<0>> == type::bitset);

  S_ASSERT(classify<std::deque<bool>> != type::bitset);
  S_ASSERT(classify<std::vector<predeclared>> != type::bitset);
  S_ASSERT(classify<std::vector<std::pair<predeclared, bool>>> != type::bitset);
}

TEST(check_tuples) {
  using namespace bxlx::graph::classification;
  struct predeclared;
  S_ASSERT(classify<std::tuple<int, float>> == type::tuple_like);
  S_ASSERT(classify<std::tuple<predeclared>> == type::tuple_like);
  S_ASSERT(classify<std::pair<const predeclared, predeclared>> == type::tuple_like);
  S_ASSERT(classify<std::pair<std::tuple<predeclared>, predeclared>> == type::tuple_like);

  S_ASSERT(classify<std::tuple<std::tuple<predeclared, const predeclared>, predeclared>> == type::tuple_like);

  S_ASSERT(classify<std::array<predeclared, 10>> != type::tuple_like);
  S_ASSERT(classify<std::array<int, 10>> != type::tuple_like);
  // S_ASSERT(classify<std::array<std::tuple<predeclared, float>, 10>> != type::tuple_like); // TODO msvc fails
}
