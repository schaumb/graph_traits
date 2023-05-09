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

template<class, class>
struct same;

template<class T>
struct same<T, T> {
  constexpr static bool value = true;
};

#define SAME(...) ASSERT(same< __VA_ARGS__ >::value)

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
  ASSERT(invalid_node_v<graph_1> == ~int{});

  using graph_2 = array<pair<string_view, string_view>, 10>;
  ASSERT(max_node_size_v<graph_2> == 20);
  ASSERT(max_edge_size_v<graph_2> == 10);
  ASSERT(invalid_node_v<graph_2> == string_view{});

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

struct edge;

constexpr bool operator<(edge const&, edge const&) {
  return false;
}

void test_check_variables() {
  using graph_1 = vector<vector<int>>;

  ASSERT(!has_graph_property_v<graph_1>);
  ASSERT(!has_node_property_v<graph_1>);
  ASSERT(!has_edge_property_v<graph_1>);
  ASSERT(!is_user_defined_node_type_v<graph_1>);
  ASSERT(has_node_container_v<graph_1>);
  ASSERT(has_adjacency_container_v<graph_1>);
  ASSERT(!has_edge_list_container_v<graph_1>);
  ASSERT(!has_edge_container_v<graph_1>);
  ASSERT(!has_in_adjacency_container_v<graph_1>);
  ASSERT(!has_in_edges_v<graph_1>);
  ASSERT(!has_invalid_node_v<graph_1>);
  // ASSERT(!is_user_defined_edge_type_v<graph_1>);
  // ASSERT(parallel_edges_v<graph_1>);
  // ASSERT(directed_edges_v<graph_1>);
  // ASSERT(compressed_edges_v<graph_1>);
  // parallel_edges_v, directed_edges_v es compressed_edges_v nincs ertelmezve graph_1 -en, igy ezek forditasi hibat okoznak

  // tovabbiakban csak az igaz allitasokat sorolom fel (vagy a letezesuket ahol van ertelme), a tobbi vagy nem letezik, vagy hamis.

  using graph_2 = pair<vector<optional<edge_prop>>, graph_prop>;
  ASSERT(has_graph_property_v<graph_2>);
  ASSERT(has_edge_property_v<graph_2>);
  ASSERT(has_adjacency_container_v<graph_2>);
  ASSERT(!parallel_edges_v<graph_2>);
  ASSERT(compressed_edges_v<graph_2>);

  using graph_3 = pair<map<string_view, node_prop>, multimap<pair<string_view, string_view>, edge_prop>>;
  ASSERT(has_node_property_v<graph_3>);
  ASSERT(has_edge_property_v<graph_3>);
  ASSERT(is_user_defined_node_type_v<graph_3>);
  ASSERT(has_node_container_v<graph_3>);
  ASSERT(has_edge_list_container_v<graph_3>);
  ASSERT(parallel_edges_v<graph_3>);
  ASSERT(directed_edges_v<graph_3>);

  using graph_4 = pair<vector<pair<list<int>, map<int, edge>>>, map<edge, edge_prop>>;
  ASSERT(has_edge_property_v<graph_4>);
  ASSERT(is_user_defined_edge_type_v<graph_4>);
  ASSERT(has_node_container_v<graph_4>);
  ASSERT(has_adjacency_container_v<graph_4>);
  ASSERT(has_edge_container_v<graph_4>);
  ASSERT(has_in_adjacency_container_v<graph_4>);
  ASSERT(has_in_edges_v<graph_4>);
  ASSERT(!parallel_edges_v<graph_4>);
  ASSERT(directed_edges_v<graph_4>);

  using graph_5 = vector<pair<size_t, deque<short>>>;
  // u.a.: using graph_5 = vector<pair<list<short>::const_iterator, list<short>>>;
  ASSERT(has_node_container_v<graph_5>);
  ASSERT(has_adjacency_container_v<graph_5>);
  ASSERT(has_in_edges_v<graph_5>);
  ASSERT(directed_edges_v<graph_5>);
}

void test_properties() {

  using graph_1 = vector<vector<int>>;
  SAME(node_t<graph_1>, int);
  SAME(edge_repr_t<graph_1>, typename vector<int>::const_iterator);
  SAME(node_container_t<graph_1>, graph_1);
  SAME(adjacency_container_t<graph_1>, vector<int>);

  using graph_2 = pair<vector<optional<edge_prop>>, graph_prop>;
  SAME(graph_property_t<graph_2>, graph_prop);
  SAME(edge_property_t<graph_2>, edge_prop);
  SAME(node_t<graph_2>, size_t);
  SAME(edge_repr_t<graph_2>, typename vector<optional<edge_prop>>::const_iterator);
  SAME(adjacency_container_t<graph_2>, vector<optional<edge_prop>>);

  using graph_3 = pair<map<string_view, node_prop>, multimap<pair<string_view, string_view>, edge_prop>>;
  SAME(node_property_t<graph_3>, node_prop);
  SAME(edge_property_t<graph_3>, edge_prop);
  SAME(node_t<graph_3>, string_view);
  SAME(edge_t<graph_3>, pair<string_view, string_view>);
  SAME(edge_repr_t<graph_3>, typename multimap<pair<string_view, string_view>, edge_prop>::const_iterator);
  SAME(node_container_t<graph_3>, map<string_view, node_prop>);
  SAME(edge_list_container_t<graph_3>, multimap<pair<string_view, string_view>, edge_prop>);

  using graph_4 = pair<vector<pair<list<int>, map<int, edge>>>, map<edge, edge_prop>>;
  SAME(edge_property_t<graph_4>, edge_prop);
  SAME(node_t<graph_4>, int);
  SAME(edge_t<graph_4>, edge);
  SAME(edge_repr_t<graph_4>, typename map<edge, edge_prop>::const_iterator);
  SAME(node_container_t<graph_4>, vector<pair<list<int>, map<int, edge>>>);
  SAME(adjacency_container_t<graph_4>, map<int, edge>);
  SAME(edge_container_t<graph_4>, map<edge, edge_prop>);
  SAME(in_adjacency_container_t<graph_4>, list<int>);

  using graph_5 = vector<pair<size_t, deque<short>>>;
  SAME(node_t<graph_5>, short);
  SAME(edge_repr_t<graph_5>, deque<short>::const_iterator);
  SAME(node_container_t<graph_5>, graph_5);
  SAME(adjacency_container_t<graph_5>, deque<short>);
}

int main() {
  test_is_graph();
  test_example_graph_representations();
  test_check_variables();
  test_properties();
}