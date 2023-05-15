//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "femto_test.hpp"
#include <bxlx/recognize/recognize.hpp>
#include <initializer_list>
#include <array>
#include <utility>
#include <tuple>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

TEST(check_input_tuple_size) {
  using namespace bxlx::graph::traits::state_machine;
  struct predeclared;
  using type_1 = std::pair<predeclared, predeclared>;
  using type_2 = std::tuple<int, char>;
  using type_3 = std::tuple<const int, predeclared&>;
  using type_4 = std::tuple<>;
  using type_5 = std::tuple<int, int, int, int, int, int>;

  S_ASSERT(is_valid_v<tuple_eq_2, type_1>);
  S_ASSERT(is_valid_v<tuple_eq_2, type_2>);
  S_ASSERT(is_valid_v<tuple_eq_2, type_3>);
  S_ASSERT(!is_valid_v<tuple_eq_2, type_4>);
  S_ASSERT(!is_valid_v<tuple_eq_2, type_5>);

  S_ASSERT(!is_valid_v<tuple_gt_2, type_1>);
  S_ASSERT(!is_valid_v<tuple_gt_2, type_2>);
  S_ASSERT(!is_valid_v<tuple_gt_2, type_3>);
  S_ASSERT(!is_valid_v<tuple_gt_2, type_4>);
  S_ASSERT(is_valid_v<tuple_gt_2, type_5>);
}

TEST(check_random_accessness) {
  struct predeclared;
  using bxlx::graph::traits::state_machine::random_access_t;

  using type_1 = const predeclared[10];
  using type_2 = std::initializer_list<predeclared>;
  using type_3 = std::array<int, 10>;
  using type_4 = std::vector<predeclared>;
  using type_5 = std::deque<bool>;
  using type_6 = std::forward_list<bool>;
  using type_7 = const std::set<predeclared>;
  using type_8 = std::unordered_multiset<predeclared>;

  S_ASSERT(random_access_t::template valid<type_1>().value);
  S_ASSERT(random_access_t::template valid<type_3>().value);
  S_ASSERT(random_access_t::template valid<type_5>().value);
  S_ASSERT(!random_access_t::template valid<type_6>().value);

#if !defined(_MSC_VER) || defined(__clang__)
  // TODO msvc fails
  S_ASSERT(random_access_t::template valid<type_2>().value);
  S_ASSERT(random_access_t::template valid<type_4>().value);
  S_ASSERT(!random_access_t::template valid<type_7>().value);
  S_ASSERT(!random_access_t::template valid<type_8>().value);
#endif
}

TEST(check_assoc_multi_property) {
  using bxlx::graph::traits::state_machine::not_multimap_t;

  using type_1 = std::map<int, int>;
  using type_2 = std::multimap<int, int>;
  using type_3 = std::unordered_map<int, int>;
  using type_4 = std::unordered_multimap<int, int>;

  S_ASSERT(not_multimap_t::template valid<type_1>().value);
  S_ASSERT(!not_multimap_t::template valid<type_2>().value);
  S_ASSERT(not_multimap_t::template valid<type_3>().value);
  S_ASSERT(!not_multimap_t::template valid<type_4>().value);
}

TEST(check_separator) {
  struct predeclared;
  using bxlx::graph::traits::state_machine::separator_of_t;

  using type_1 = const predeclared[10];
  using type_2 = std::initializer_list<predeclared>;
  using type_3 = std::array<int, 10>;
  using type_4 = std::vector<predeclared>;
  using type_5 = std::deque<bool>;
  using type_6 = std::forward_list<bool>;
  using type_7 = std::list<predeclared>;
  using type_8 = const std::set<predeclared>;
  using type_9 = std::unordered_multiset<predeclared>;


  S_ASSERT(separator_of_t::template valid<std::pair<int, type_1>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<const predeclared*, type_1>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<int, type_2>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<const predeclared*, type_2>>().value);
  // TODO S_ASSERT(separator_of_t::template valid<std::pair<int, type_3>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<const int*, type_3>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<int, type_4>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<typename type_4::iterator, type_4>>().value);
  // TODO MSVC S_ASSERT(separator_of_t::template valid<std::pair<int, type_5>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<typename type_5::iterator, type_5>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<int, type_6>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<typename type_6::iterator, type_6>>().value);
  S_ASSERT(!separator_of_t::template valid<std::pair<int, type_7>>().value);
  // TODO S_ASSERT(separator_of_t::template valid<std::pair<typename type_7::iterator, type_7>>().value);
  // TODO S_ASSERT(!separator_of_t::template valid<std::pair<int, type_8>>().value);
  // TODO S_ASSERT(separator_of_t::template valid<std::pair<typename type_8::const_iterator, type_8>>().value);
  // TODO S_ASSERT(!separator_of_t::template valid<std::pair<int, type_9>>().value);
  // TODO S_ASSERT(!separator_of_t::template valid<std::pair<typename type_9::iterator, type_9>>().value);
}

TEST(check_empty_properties) {
  using namespace bxlx::graph::traits::properties;

  S_ASSERT(!has_property_v<empty_t, struct any>);
  SAME(get_value_t<empty_t, struct any, std::enable_if<true, struct any>>, struct any);
}

TEST(check_multi_properties) {
  using namespace bxlx::graph::traits::properties;

  using props = merge_properties_t<
    property_t<struct s1, struct v1>,
    property_t<struct s2, struct v2>,
    property_t<struct s3, struct v1>
  >;

  S_ASSERT(has_property_v<props, struct s1>);
  S_ASSERT(has_property_v<props, struct s2>);
  S_ASSERT(has_property_v<props, struct s3>);
  S_ASSERT(!has_property_v<props, struct s4>);
  S_ASSERT(!has_property_v<props, struct v1>);

  SAME(get_value_t<props, struct s1>, struct v1);
  SAME(get_value_t<props, struct s2>, struct v2);
  SAME(get_value_t<props, struct s3>, struct v1);
}
