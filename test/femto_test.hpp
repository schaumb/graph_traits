//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_FEMTO_TEST_HPP
#define BXLX_GRAPH_FEMTO_TEST_HPP

#include <set>
#include <cassert>
#include <cstdlib>
#include <stdexcept>

namespace test {
template<class = void>
std::set<void (*)()>& TESTCASES() { static std::set<void (*)()> TCS; return TCS; }

template<void (*V)()>
const bool TEST_REG = ::test::TESTCASES().insert(V).second;

template<class = void>
void run() { for (auto v : ::test::TESTCASES()) v(); }

struct assert_error : std::logic_error { using std::logic_error::logic_error; };
}

#define TEST(name) void name(); template const bool ::test::TEST_REG<&name>; void name()

#define STRINGIZE_1(x) #x
#define STRINGIZE_2(x) STRINGIZE_1(x)
#define ASSERT(...)                                                                                             \
  while ((assert((__VA_ARGS__)), !(__VA_ARGS__)))                                                               \
    throw test::assert_error(__FILE__ ":" STRINGIZE_2(__LINE__)  ": void " + std::string{__FUNCTION__} + "(): " \
                             "Assertion `(" #__VA_ARGS__  ")' failed.")
#define S_ASSERT(...) static_assert((__VA_ARGS__))
#define SAME(...) S_ASSERT(std::is_same_v<__VA_ARGS__>)

#endif //BXLX_GRAPH_FEMTO_TEST_HPP