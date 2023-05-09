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
#  define STRINGIZE_1(x) #x
#  define STRINGIZE_2(x) STRINGIZE_1(x)
#  define ASSERT(...) static_assert((__VA_ARGS__))
#  define RASSERT(...)                                                                                                 \
    assert((__VA_ARGS__));                                                                                             \
    while (!(__VA_ARGS__))                                                                                             \
    throw std::logic_error(__FILE__ ":" STRINGIZE_2(__LINE__) " : " #__VA_ARGS__)

#else
#  define STRINGIZE_1(x) #x
#  define STRINGIZE_2(x) STRINGIZE_1(x)
#  define ASSERT(...)                                                                                                  \
    assert((__VA_ARGS__));                                                                                             \
    while (!(__VA_ARGS__))                                                                                             \
    throw std::logic_error(__FILE__ ":" STRINGIZE_2(__LINE__) " : " #__VA_ARGS__)
#  define RASSERT(...) ASSERT(__VA_ARGS__)
#endif

template<class, class>
struct same;

template<class T>
struct same<T, T> {
  constexpr static bool value = true;
};

#define SAME(...) ASSERT(same< __VA_ARGS__ >::value)

#define THROWS(...) try {            \
    __VA_ARGS__;                     \
    RASSERT(false && "Not thrown: " # __VA_ARGS__); \
  } catch (...) {}

static_assert(bxlx::graph::version >= "1.0.0");
static_assert(bxlx::graph::major_version >= 1);
static_assert(!bxlx::graph::is_graph_v<void>);

struct edge_prop;
struct node_prop;
struct graph_prop;

using namespace bxlx::graph;
using namespace std;


void test_is_graph() {
  constexpr auto adjacency_list   = representation_t::adjacency_list;
  constexpr auto adjacency_matrix = representation_t::adjacency_matrix;
  constexpr auto edge_list        = representation_t::edge_list;

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
  constexpr auto adjacency_list   = representation_t::adjacency_list;
  constexpr auto adjacency_matrix = representation_t::adjacency_matrix;
  constexpr auto edge_list        = representation_t::edge_list;
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
  SAME(edge_repr_t<graph_1>, typename vector<int>::iterator);
  SAME(node_container_t<graph_1>, graph_1);
  SAME(adjacency_container_t<graph_1>, vector<int>);

  using graph_2 = pair<vector<optional<edge_prop>>, graph_prop>;
  SAME(graph_property_t<graph_2>, graph_prop);
  SAME(edge_property_t<graph_2>, edge_prop);
  SAME(node_t<graph_2>, size_t);
  SAME(edge_repr_t<graph_2>, typename vector<optional<edge_prop>>::iterator);
  SAME(adjacency_container_t<graph_2>, vector<optional<edge_prop>>);

  using graph_3 = const pair<map<string_view, node_prop>, multimap<pair<string_view, string_view>, edge_prop>>;
  SAME(node_property_t<graph_3>, const node_prop);
  SAME(edge_property_t<graph_3>, const edge_prop);
  SAME(node_t<graph_3>, string_view);
  SAME(edge_t<graph_3>, pair<string_view, string_view>);
  SAME(edge_repr_t<graph_3>, typename multimap<pair<string_view, string_view>, edge_prop>::const_iterator);
  SAME(node_container_t<graph_3>, const map<string_view, node_prop>);
  SAME(edge_list_container_t<graph_3>, const multimap<pair<string_view, string_view>, edge_prop>);

  using graph_4 = pair<vector<pair<list<int>, map<int, edge>>>, map<edge, edge_prop>>;
  SAME(edge_property_t<graph_4>, edge_prop);
  SAME(node_t<graph_4>, int);
  SAME(edge_t<graph_4>, edge);
  SAME(edge_repr_t<graph_4>, typename map<edge, edge_prop>::iterator);
  SAME(node_container_t<graph_4>, vector<pair<list<int>, map<int, edge>>>);
  SAME(adjacency_container_t<graph_4>, map<int, edge>);
  SAME(edge_container_t<graph_4>, map<edge, edge_prop>);
  SAME(in_adjacency_container_t<graph_4>, list<int>);

  using graph_5 = vector<pair<size_t, deque<short>>>;
  SAME(node_t<graph_5>, short);
  SAME(edge_repr_t<graph_5>, deque<short>::iterator);
  SAME(node_container_t<graph_5>, graph_5);
  SAME(adjacency_container_t<graph_5>, deque<short>);
}

void test_graph_getters() {
  tuple<vector<string_view>, vector<tuple<int, int, int>>,
        vector<string_view>, string_view> graph_1 {
              {"node_0", "node_1"}, {{0, 1, 0}}, {"edge_0"}, "graph_prop"
        }; // undirected because it has edge property sharing.

  vector<tuple<vector<int>, map<int, double>, float>> graph_2 {
        { {}, {{1, 1.0}}, 2.0f },
        { {0}, {}, 3.0f }
  };

  RASSERT(&nodes(graph_1) == &std::get<0>(graph_1));
  RASSERT(&edges(graph_1) == &std::get<2>(graph_1));
  RASSERT(&edge_list(graph_1) == &std::get<1>(graph_1));
  RASSERT(graph_property(graph_1) == "graph_prop");

  RASSERT(adjacents(&graph_2, 0) != nullptr);
  RASSERT(adjacents(&graph_2, 2) == nullptr);
  RASSERT(&adjacents(graph_2, 0) == &std::get<1>(graph_2[0]));
  THROWS(adjacents(graph_2, 2));

  RASSERT(in_adjacents(&graph_2, 0) != nullptr);
  RASSERT(in_adjacents(&graph_2, 2) == nullptr);
  RASSERT(&in_adjacents(graph_2, 0) == &std::get<0>(graph_2[0]));
  THROWS(in_adjacents(graph_2, 2));

  RASSERT(node_property(graph_1, 0) == "node_0");

  RASSERT(node_property(&graph_2, 0) != nullptr);
  RASSERT(node_property(&graph_2, 2) == nullptr);
  RASSERT(node_property(graph_2, 0) == 2.0f);
  THROWS(node_property(graph_2, 2));

  RASSERT(get_edge(graph_1, 0) != invalid_edge(graph_1));
  RASSERT(get_adjacency(graph_1, 0, 1) != invalid_edge(graph_1));
  RASSERT(get_adjacency(graph_1, 1, 0) == get_adjacency(graph_1, 0, 1));
  // get_edge(graph_1, 0, 1);
  // undefined --> undirected, but not contains all edge both direction

  RASSERT(edge_property(&graph_1, 0) != nullptr);
  RASSERT(edge_property(graph_1, 0) == "edge_0");
  RASSERT(edge_property(graph_1, 0, 1) == "edge_0");
  RASSERT(edge_property(graph_1, get_edge(graph_1, 0)) == "edge_0");

  RASSERT(get_edge(graph_2, 0, 1) != invalid_edge(graph_2));
  RASSERT(get_edge(graph_2, 1, 0) == invalid_edge(graph_2));
  RASSERT(get_edge(graph_2, 0, 2) == invalid_edge(graph_2));
  RASSERT(get_adjacency(graph_2, 1, 0) != get_adjacency(graph_2, 0, 1));
  RASSERT(get_adjacency(graph_2, 1, 0) == invalid_edge(graph_2));

  RASSERT(edge_property(&graph_2, 0, 1) != nullptr);
  RASSERT(edge_property(&graph_2, 0, 2) == nullptr);
  RASSERT(edge_property(graph_2, 0, 1) == 1.0);
  THROWS(edge_property(graph_2, 0, 2));

  RASSERT(edge_property(graph_2, get_edge(graph_2, 0, 1)) == 1.0);
}
#include <iostream>
void test_query() {

  bitset<36> graph_1 {}; // compressed, directed
  graph_1[1] = true; // 0 -> 1 edge
  graph_1[8] = true; // 1 -> 2 edge

  list<pair<string_view, string_view>> graph_2 {
        {"node_1", "node_2"},
        {"node_5", "node_2"},
        {"node_2", "node_5"},
  };

  RASSERT(has_node(graph_1, 0));
  RASSERT(!has_node(graph_1, 6));
  RASSERT(has_node(graph_2, "node_5"));
  RASSERT(!has_node(graph_2, "no_node"));

  RASSERT(has_edge(graph_1, 0, 1));
  RASSERT(!has_edge(graph_1, 2, 1));
  RASSERT(!has_edge(graph_2, "node_2", "node_1"));

  // on these graph the has_adjacency is the same as has_edge

  RASSERT(node_count(graph_1) == 6);
  RASSERT(edge_count(graph_1) == 2);
  RASSERT(adjacency_count(graph_1) == 2);

  RASSERT(node_count(graph_2) == 3);
  RASSERT(adjacency_count(graph_2) == 3);

  // edge_count(graph_2)
  // undefined --> not exact directed/undirected edges. No property/shared property used

  RASSERT(size(out_edges(graph_1, 0)) == 1);
  RASSERT(size(out_edges(graph_2, "node_2")) == 1);
  RASSERT(size(in_edges(graph_2, "node_2")) == 2);

  for (auto&& [node, repr] : out_edges(graph_2, "node_2")) {
    RASSERT(node == "node_5");
    RASSERT(repr != invalid_edge(graph_2));
  }
}

int main() {
  test_is_graph();
  test_example_graph_representations();
  test_check_variables();
  test_properties();
  test_graph_getters();
  test_query();
}