#include <bxlx/graph>

#include <vector>
#include <list>
#include <tuple>
#include <optional>
#include <cassert>

#ifdef CEXPR_ASSERT_TEST
#define ASSERT(...) static_assert((__VA_ARGS__))
#else
#define ASSERT(...) assert((__VA_ARGS__))
#endif

struct edge_prop;
struct node_prop;
struct graph_prop;

int main() {
    using namespace bxlx::graph;
    using namespace std;

    constexpr auto adjacency_list = graph_representation_t::adjacency_list;
    constexpr auto adjacency_matrix = graph_representation_t::adjacency_matrix;
    constexpr auto edge_list = graph_representation_t::edge_list;

    ASSERT(graph_representation<vector<vector<int>>> == adjacency_list);

    ASSERT(graph_representation<bool[10][10]> == adjacency_matrix);

    ASSERT(graph_representation<list<tuple<int, int, int>>> ==
    edge_list);

    ASSERT(graph_representation<
        tuple<vector<pair<list<optional<edge_prop>>, node_prop>>, graph_prop>
    > == adjacency_matrix);
}