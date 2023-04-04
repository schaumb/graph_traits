#include <bxlx/graph>

#include <bxlx/classify/type_classification.hpp>

#include <array>
#include <atomic>
#include <cassert>
#include <deque>
#include <forward_list>
#include <list>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
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

using namespace bxlx::graph;
using namespace std;

constexpr auto adjacency_list   = representation_t::adjacency_list;
constexpr auto adjacency_matrix = representation_t::adjacency_matrix;
constexpr auto edge_list        = representation_t::edge_list;

void           test_example_graph_representations() {
  ASSERT(representation<vector<vector<int>>> == adjacency_list);

  ASSERT(representation<bool[10][10]> == adjacency_matrix);

  ASSERT(representation<list<tuple<int, int, int>>> == edge_list);

  using Mat = tuple<vector<pair<list<optional<edge_prop>>, node_prop>>, graph_prop>;
  ASSERT(representation<Mat> == adjacency_matrix);
}


int main() {
  // test_example_graph_representations();

  using namespace bxlx::graph::type_classification;
  using T = std::tuple<int, double>;
  static_assert(std::is_same_v<typename detail::template_inspect<T>::types, T>);
  static_assert(
        std::is_same_v<std::tuple_element_t<0, typename detail::template_inspect<std::deque<double>>::types>, double>);
  static_assert(
        std::is_same_v<std::tuple_element_t<0, typename detail::template_inspect<std::vector<double>>::types>, double>);
  static_assert(std::is_same_v<std::tuple_element_t<0, typename detail::template_inspect<std::optional<double>>::types>,
                               double>);
  static_assert(
        std::is_same_v<std::tuple_element_t<0, typename detail::template_inspect<std::forward_list<double>>::types>,
                       double>);
  static_assert(
        std::is_same_v<std::tuple_element_t<0, typename detail::template_inspect<std::atomic<double>>::types>, double>);
  static_assert(std::is_same_v<std::tuple_element_t<0, typename detail::template_inspect<std::array<double, 6>>::types>,
                               double>);
  static_assert(std::is_same_v<
                std::tuple_element_t<0, typename detail::template_inspect<std::unordered_map<double, double>>::types>,
                double>);
  static_assert(
        std::is_same_v<
              std::tuple_element_t<1, typename detail::template_inspect<std::unordered_map<double, int>>::types>, int>);
}