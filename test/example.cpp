#include <bxlx/graph>

#include <cassert>
#include <list>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <vector>

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

int main() {
  using namespace bxlx::graph;
  using namespace std;

  constexpr auto adjacency_list   = graph_representation_t::adjacency_list;
  constexpr auto adjacency_matrix = graph_representation_t::adjacency_matrix;
  constexpr auto edge_list        = graph_representation_t::edge_list;

  ASSERT(graph_representation<vector<vector<int>>> == adjacency_list);

  ASSERT(graph_representation<bool[10][10]> == adjacency_matrix);

  ASSERT(graph_representation<list<tuple<int, int, int>>> == edge_list);

  using Mat = tuple<vector<pair<list<optional<edge_prop>>, node_prop>>, graph_prop>;
  ASSERT(graph_representation<Mat> == adjacency_matrix);
}