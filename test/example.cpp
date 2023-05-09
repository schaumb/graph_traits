#include <bxlx/classify/optional_traits.hpp>
#include <bxlx/classify/range_traits.hpp>
#include <bxlx/classify/type_traits.hpp>
#include <bxlx/recognize/recognize.hpp>
#include <bxlx/graph>

#include <array>
#include <atomic>
#include <bitset>
#include <cassert>
#include <deque>
#include <forward_list>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>


#define CEXPR_ASSERT_TEST
#ifdef CEXPR_ASSERT_TEST
#  define ASSERT(...) static_assert((__VA_ARGS__))
#else
#  define STRINGIZE_1(x) #x
#  define STRINGIZE_2(x) STRINGIZE_1(x)
#  define ASSERT(...)                                                                                                  \
    assert((__VA_ARGS__));                                                                                             \
    while (!(__VA_ARGS__))                                                                                             \
    throw std::logic_error(__FILE__ ":" STRINGIZE_2(__LINE__) " : " #__VA_ARGS__)
#endif

static_assert(bxlx::graph::version >= "1.0.0");
static_assert(bxlx::graph::major_version >= 1);
static_assert(!bxlx::graph::is_graph_v<void>);

struct edge_prop;
struct node_prop;
struct graph_prop;

using namespace bxlx::graph;
using namespace std;

constexpr auto adjacency_list   = representation_t::adjacency_list;
constexpr auto adjacency_matrix = representation_t::adjacency_matrix;
constexpr auto edge_list        = representation_t::edge_list;

void test_is_graph() {

  using graph_1 = vector<list<int>>;
  using graph_2 = bitset<36>;
  using graph_3 = forward_list<tuple<string, string>>;
  using n_graph = vector<float>;

  ASSERT( is_graph_v<graph_1>);
  ASSERT( is_graph_v<graph_2>);
  ASSERT( is_graph_v<graph_3>);
  ASSERT(!is_graph_v<n_graph>);

  ASSERT(representation_v<graph_1> == adjacency_list);
  ASSERT(representation_v<graph_2> == adjacency_matrix);
  ASSERT(representation_v<graph_3> == edge_list);

  // ASSERT(!it_is_a_graph_v<n_graph>); // compile error inside template instantiation
}

void test_constants() {
  using graph_1 = int[10][9];
  ASSERT(max_node_size_v<graph_1> == 10);
  ASSERT(max_edge_size_v<graph_1> == 90);
  //ASSERT(invalid_node_v<graph_1> == ~int{});

  using graph_2 = array<pair<string_view, string_view>, 10>;
  ASSERT(max_node_size_v<graph_2> == 20);
  ASSERT(max_edge_size_v<graph_2> == 10);
  //ASSERT(invalid_node_v<graph_2> == string_view{});

  using graph_3 = bitset<36>;
  ASSERT(max_node_size_v<graph_3> == 6);
  ASSERT(max_edge_size_v<graph_3> == 36);

  using graph_4 = array<multimap<uint8_t, float>, 100>;
  ASSERT(max_node_size_v<graph_4> == 100);
  ASSERT(max_edge_size_v<graph_4> == (size_t)(-1));

  using graph_5 = unordered_map<string_view, unordered_set<string_view>>;
  ASSERT(max_node_size_v<graph_5> == (size_t)(-1));
  ASSERT(max_edge_size_v<graph_5> == (size_t)(-1));
}

void test_example_graph_representations() {
  ASSERT(representation_v<vector<vector<int>>> == adjacency_list);

  ASSERT(representation_v<bool[10][10]> == adjacency_matrix);

  ASSERT(representation_v<list<tuple<int, int, int>>> == edge_list);

  using Mat = tuple<vector<pair<vector<optional<edge_prop>>, node_prop>>, graph_prop>;
  ASSERT(representation_v<Mat> == adjacency_matrix);
}


int main() {
  test_is_graph();
  test_example_graph_representations();

}