#include <bxlx/graph>
#include <bxlx/classify/type_traits.hpp>
#include <bxlx/classify/range_traits.hpp>
#include <bxlx/classify/optional_traits.hpp>

#include <array>
#include <map>
#include <atomic>
#include <bitset>
#include <cassert>
#include <deque>
#include <forward_list>
#include <list>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <vector>

#define CEXPR_ASSERT_TEST
#ifdef CEXPR_ASSERT_TEST
#  define ASSERT(...) static_assert((__VA_ARGS__))
#else
#  define STRINGIZE_1(x) #  x
#  define STRINGIZE_2(x) STRINGIZE_1(x)
#  define ASSERT(...)                                                                                                  \
    assert((__VA_ARGS__));                                                                                             \
    while (!(__VA_ARGS__))                                                                                             \
    throw std::logic_error(__FILE__ ":" STRINGIZE_2(__LINE__) " : " #__VA_ARGS__)
#endif

struct edge_prop;
struct node_prop;
struct graph_prop;

using namespace bxlx::graph;
using namespace std;

constexpr auto adjacency_list   = representation_t::adjacency_list;
constexpr auto adjacency_matrix = representation_t::adjacency_matrix;
constexpr auto edge_list        = representation_t::edge_list;

void test_example_graph_representations() { /*
  ASSERT(representation<vector<vector<int>>> == adjacency_list);

  ASSERT(representation<bool[10][10]> == adjacency_matrix);

  ASSERT(representation<list<tuple<int, int, int>>> == edge_list);

  using Mat = tuple<vector<pair<list<optional<edge_prop>>, node_prop>>, graph_prop>;
  ASSERT(representation<Mat> == adjacency_matrix);*/
}

template<class T>
using templates = typename bxlx::graph::type_classification::detail::template_inspect<T>::types;

template<class Check, bool is_range = false>
void check_is_tuple() {
  using namespace bxlx::graph::type_classification;
  ASSERT(is_tuple_v<Check>);
  ASSERT(!is_optional_v<Check>);
  ASSERT(is_range == is_range_v<Check>);
  ASSERT(!is_bool_v<Check>);
  ASSERT(!is_char_v<Check>);
  ASSERT(!is_index_v<Check>);
  ASSERT(!is_bitset_v<Check>);
}


template<class Check>
void check_is_optional() {
  using namespace bxlx::graph::type_classification;
  ASSERT(!is_tuple_v<Check>);
  ASSERT(is_optional_v<Check>);
  ASSERT(!is_range_v<Check>);
  ASSERT(!is_bool_v<Check>);
  ASSERT(!is_char_v<Check>);
  ASSERT(!is_index_v<Check>);
  ASSERT(!is_bitset_v<Check>);
}

template<class Check>
void check_is_range() {
  using namespace bxlx::graph::type_classification;
  ASSERT(!is_tuple_v<Check>);
  ASSERT(!is_optional_v<Check>);
  ASSERT(is_range_v<Check>);
  ASSERT(!is_bool_v<Check>);
  ASSERT(!is_char_v<Check>);
  ASSERT(!is_index_v<Check>);
  ASSERT(!is_bitset_v<Check>);
}

template<class Check, bool is_range = false>
void check_is_bitset() {
  using namespace bxlx::graph::type_classification;
  ASSERT(!is_tuple_v<Check>);
  ASSERT(!is_optional_v<Check>);
  ASSERT(is_range == is_range_v<Check>);
  ASSERT(!is_bool_v<Check>);
  ASSERT(!is_char_v<Check>);
  ASSERT(!is_index_v<Check>);
  ASSERT(is_bitset_v<Check>);
}


void test_type_traits() {
  using namespace bxlx::graph::type_classification;
  check_is_tuple<tuple<int>>();
  check_is_tuple<tuple<int, float, edge_prop>>();
  check_is_tuple<pair<int, edge_prop>>();
  check_is_tuple<array<int, 10>, true>();
  ASSERT(!is_tuple_v<tuple<>>);
  ASSERT(!is_tuple_v<array<int, 0>>);
  ASSERT(!is_tuple_v<unordered_map<int, int>>);
  ASSERT(!is_tuple_v<int>);
  ASSERT(!is_tuple_v<int[10]>);
  ASSERT(!is_tuple_v<int(&)[10]>);

  ASSERT(is_same_v<templates<tuple<int, edge_prop, double>>, tuple<int, edge_prop, double>>);
  ASSERT(is_same_v<templates<pair<int, edge_prop>>, tuple<int, edge_prop>>);
  ASSERT(is_same_v<tuple_element_t<0, templates<vector<edge_prop>>>, edge_prop>);
  ASSERT(is_same_v<tuple_element_t<0, templates<atomic<bool>>>, bool>);
  ASSERT(is_same_v<tuple_element_t<0, templates<deque<edge_prop>>>, edge_prop>);
  ASSERT(is_same_v<tuple_element_t<0, templates<array<edge_prop, 6>>>, edge_prop>);
  ASSERT(tuple_element_t<1, templates<array<edge_prop, 6>>>{}() == 6);
  ASSERT(is_same_v<tuple_element_t<0, templates<unordered_map<edge_prop, float>>>, edge_prop>);
  ASSERT(is_same_v<tuple_element_t<1, templates<unordered_map<int, edge_prop>>>, edge_prop>);
  ASSERT(is_same_v<templates<edge_prop>, tuple<>>);
  ASSERT(is_same_v<templates<int[10]>, tuple<>>);

  ASSERT(!detail::is_defined_v<edge_prop>);
  ASSERT(!detail::is_defined_v<node_prop*>);
  ASSERT(!detail::is_defined_v<optional<node_prop>>);
  ASSERT(!detail::is_defined_v<tuple<int, node_prop, double>>);
  ASSERT(!detail::is_defined_v<pair<int, node_prop>>);

  ASSERT(detail::is_defined_v<vector<edge_prop>>);
  // ASSERT(detail::is_defined_v<deque<edge_prop>>);
  // ASSERT(!detail::is_defined_v<unordered_map<int, edge_prop>>);

  check_is_optional<optional<node_prop>>();
  check_is_optional<node_prop*>();
  check_is_optional<const volatile node_prop* const>();
  check_is_optional<const std::optional<int>>();
  ASSERT(!is_optional_v<int[10]>);
  ASSERT(!is_optional_v<int[]>);
  ASSERT(!is_optional_v<node_prop[]>);
  ASSERT(!is_optional_v<const bitset<10>>);

  ASSERT(is_same_v<optional_value_t<optional<node_prop>>, node_prop>);
  ASSERT(is_same_v<optional_reference_t<const volatile node_prop* const>, const volatile node_prop&>);

  check_is_range<vector<int>>();
  check_is_range<vector<node_prop>>();
  check_is_range<deque<bool>>();
  check_is_range<deque<node_prop>>();
  check_is_range<unordered_map<int, edge_prop>>();
  check_is_range<map<edge_prop, node_prop>>();

  ASSERT(is_bool_v<bool>);
  ASSERT(is_bool_v<bitset<10>::reference>);
  ASSERT(is_bool_v<vector<bool>::reference>);
  ASSERT(!is_bool_v<std::reference_wrapper<bool>>);
  ASSERT(!is_bool_v<std::atomic<bool>>);

  ASSERT(is_char_v<volatile wchar_t>);
  ASSERT(!is_char_v<std::uint8_t>);

  enum ASDASD{};
  ASSERT(is_index_v<std::size_t>);
  ASSERT(is_index_v<std::atomic<int>>);
  ASSERT(is_index_v<ASDASD>);

  ASSERT(!is_index_v<int&>);
  ASSERT(!is_index_v<char>);
  ASSERT(!is_index_v<bool>);


  check_is_bitset<const bitset<10>>();
  check_is_bitset<vector<bool>, true>();
  ASSERT(!is_bitset_v<vector<edge_prop>>);
  ASSERT(!is_bitset_v<optional<edge_prop>>);
  ASSERT(!is_bitset_v<unordered_map<int, edge_prop>>);

}

int main() {
  // test_example_graph_representations();
  test_type_traits();
}