#include <bxlx/classify/optional_traits.hpp>
#include <bxlx/classify/range_traits.hpp>
#include <bxlx/classify/type_traits.hpp>
#include <bxlx/recognize/recognize.hpp>
#include <bxlx/graph>

#include <array>
#include <atomic>
#include <bitset>
/*
#include <boost/bimap/bimap.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/optional.hpp>
#include <boost/poly_collection/any_collection.hpp>
#include <boost/type_erasure/operators.hpp>
 */
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

struct edge_prop;
struct node_prop;
struct graph_prop;

using namespace bxlx::graph;
using namespace std;

constexpr auto adjacency_list   = representation_t::adjacency_list;
constexpr auto adjacency_matrix = representation_t::adjacency_matrix;
constexpr auto edge_list        = representation_t::edge_list;

void test_example_graph_representations() {
  ASSERT(representation_v<vector<vector<int>>> == adjacency_list);

  ASSERT(representation_v<bool[10][10]> == adjacency_matrix);

  ASSERT(representation_v<list<tuple<int, int, int>>> == edge_list);

  using Mat = tuple<vector<pair<vector<optional<edge_prop>>, node_prop>>, graph_prop>;
  ASSERT(representation_v<Mat> == adjacency_matrix);
}

template <class T>
using templates = typename bxlx::graph::type_traits::detail::template_inspect<T>::types;

template <class Check, bool is_range = false>
void check_is_tuple() {
  using namespace bxlx::graph::type_traits;
  ASSERT(is_tuple_v<Check>);
  ASSERT(!is_optional_v<Check>);
  ASSERT(is_range == is_range_v<Check>);
  ASSERT(!is_bool_v<Check>);
  ASSERT(!is_index_v<Check>);
  ASSERT(!is_bitset_v<Check>);
}


template <class Check>
void check_is_optional() {
  using namespace bxlx::graph::type_traits;
  ASSERT(!is_tuple_v<Check>);
  ASSERT(is_optional_v<Check>);
  ASSERT(!is_range_v<Check>);
  ASSERT(!is_bool_v<Check>);
  ASSERT(!is_index_v<Check>);
  ASSERT(!is_bitset_v<Check>);
}

template <class Check>
void check_is_range() {
  using namespace bxlx::graph::type_traits;
  ASSERT(!is_tuple_v<Check>);
  ASSERT(!is_optional_v<Check>);
  ASSERT(is_range_v<Check>);
  ASSERT(!is_bool_v<Check>);
  ASSERT(!is_index_v<Check>);
  ASSERT(!is_bitset_v<Check>);
}

template <class Check, bool is_range = false>
void check_is_bitset() {
  using namespace bxlx::graph::type_traits;
  ASSERT(!is_tuple_v<Check>);
  ASSERT(!is_optional_v<Check>);
  ASSERT(is_range == is_range_v<Check>);
  ASSERT(!is_bool_v<Check>);
  ASSERT(!is_index_v<Check>);
  ASSERT(is_bitset_v<Check>);
}


template <class Check, class Reference, class IteratorTag, bool St, bxlx::graph::type_traits::range_type_t Rt>
void check_range_type() {
  using namespace bxlx::graph::type_traits;
  check_is_range<Check>();
  ASSERT(std::is_same_v<range_reference_t<Check>, Reference>);
  ASSERT(std::is_base_of_v<IteratorTag, range_iterator_tag_t<Check>>);
  ASSERT(St == range_is_continuous_v<Check>);
  ASSERT(Rt == range_type_v<Check>);
}


void test_type_traits() {
  using namespace bxlx::graph::type_traits;
  check_is_tuple<tuple<int>>();
  check_is_tuple<tuple<int, float, edge_prop>>();
  check_is_tuple<pair<int, edge_prop>>();
  check_is_tuple<array<int, 10>, true>();
  check_is_tuple<const array<int, 10>, true>();
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

  ASSERT(!is_defined_v<edge_prop>);
  ASSERT(!is_defined_v<node_prop*>);
  ASSERT(!is_defined_v<optional<node_prop>>);
  ASSERT(!is_defined_v<tuple<int, node_prop, double>>);
  ASSERT(!is_defined_v<pair<int, node_prop>>);

  ASSERT(is_defined_v<vector<edge_prop>>);

  // These are different on platforms
  // ASSERT(is_defined_v<deque<edge_prop>>);
  // ASSERT(!is_defined_v<unordered_map<int, edge_prop>>);

  check_is_optional<optional<node_prop>>();
  check_is_optional<node_prop*>();
  check_is_optional<const volatile node_prop* const>();
  check_is_optional<const std::optional<int>>();
  check_is_optional<const std::optional<struct undef>>();

  class A;
  check_is_optional<std::unique_ptr<A>>();
  check_is_optional<std::shared_ptr<A>>();
  // check_is_optional<std::shared_ptr<A[]>>();
  ASSERT(is_defined_v<void()>);
  ASSERT(!is_optional_v<std::function<void()>>);
  ASSERT(!is_optional_v<int[10]>);
  ASSERT(!is_optional_v<int[]>);
  ASSERT(!is_optional_v<node_prop[]>);
  ASSERT(!is_optional_v<const bitset<10>>);

  ASSERT(is_same_v<optional_value_t<optional<node_prop>>, node_prop>);
  ASSERT(is_same_v<optional_reference_t<const volatile node_prop* const>, const volatile node_prop&>);

  check_is_range<vector<int>>();
  check_is_range<const vector<node_prop>>();
  check_is_range<deque<bool>>();
  check_is_range<deque<node_prop>>();
  check_is_range<unordered_map<int, edge_prop>>();
  check_is_range<const map<edge_prop, node_prop>>();
  check_is_range<map<edge_prop, int>>();

  ASSERT(is_bool_v<bool>);
  ASSERT(is_bool_v<bitset<10>::reference>);
  ASSERT(is_bool_v<vector<bool>::reference>);
  ASSERT(!is_bool_v<std::reference_wrapper<bool>>);
  ASSERT(is_bool_v<std::atomic<bool>>);

  enum ASDASD {};
  ASSERT(is_index_v<std::size_t>);
  ASSERT(is_index_v<std::atomic<int>>);
  ASSERT(!is_index_v<ASDASD>);

  ASSERT(!is_index_v<int&>);
  ASSERT(!is_index_v<char>);
  ASSERT(!is_index_v<bool>);


  check_is_bitset<const bitset<10>>();
  check_is_bitset<vector<bool>, true>();
  ASSERT(!is_bitset_v<vector<edge_prop>>);
  ASSERT(!is_bitset_v<optional<edge_prop>>);
  ASSERT(!is_bitset_v<unordered_map<int, edge_prop>>);


  check_range_type<std::vector<edge_prop>, edge_prop&, std::random_access_iterator_tag, true, range_type_t::sequence>();

  check_range_type<const std::deque<edge_prop>, const edge_prop&, std::random_access_iterator_tag, false,
                   range_type_t::queue_like>();

  check_range_type<std::forward_list<edge_prop>, edge_prop&, std::forward_iterator_tag, false,
                   range_type_t::sequence>();

  check_range_type<std::list<int>, int&, std::bidirectional_iterator_tag, false, range_type_t::queue_like>();

  check_range_type<std::set<std::pair<int, node_prop>>, const std::pair<int, node_prop>&,
                   std::bidirectional_iterator_tag, false, range_type_t::set_like>();

  check_range_type<std::map<node_prop, int>, std::pair<const node_prop, int>&, std::bidirectional_iterator_tag, false,
                   range_type_t::map_like>();

  check_range_type<std::unordered_set<node_prop>, const node_prop&, std::forward_iterator_tag, false,
                   range_type_t::set_like>();

  check_range_type<std::unordered_set<std::pair<node_prop, int>>, const std::pair<node_prop, int>&,
                   std::forward_iterator_tag, false, range_type_t::set_like>();

  check_range_type<std::unordered_map<node_prop, int>, std::pair<const node_prop, int>&, std::forward_iterator_tag,
                   false, range_type_t::map_like>();

  ASSERT(!is_associative_multi_v<std::set<node_prop>>);
  ASSERT(!is_associative_multi_v<std::map<node_prop, int>>);
  ASSERT(!is_associative_multi_v<std::unordered_set<node_prop>>);
  ASSERT(!is_associative_multi_v<std::unordered_map<node_prop, int>>);

  ASSERT(is_associative_multi_v<std::multiset<node_prop>>);
  ASSERT(is_associative_multi_v<std::multimap<node_prop, int>>);
  ASSERT(is_associative_multi_v<std::unordered_multiset<node_prop>>);
  ASSERT(is_associative_multi_v<std::unordered_multimap<node_prop, int>>);
  /*
#include <boost/optional.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/poly_collection/any_collection.hpp>
  struct undef;
  check_is_optional<boost::optional<undef>>();
  ASSERT(!is_bool_v<boost::logic::tribool>);
  ASSERT(!is_index_v<boost::logic::tribool>);
  using renderable      = boost::type_erasure::ostreamable<>;
  using poly_collection = boost::poly_collection::any_collection<renderable>;
  check_range_type<poly_collection, typename poly_collection::reference, std::forward_iterator_tag, false,
                   range_type_t::sequence>();

  check_range_type<boost::container::flat_set<undef>, undef&, std::random_access_iterator_tag, true,
                   range_type_t::set_like>();
  check_range_type<boost::container::flat_multimap<undef, undef>, std::pair<undef, undef>&,
                   std::random_access_iterator_tag,
                   false, // this need to be true
                   range_type_t::map_like>();

  ASSERT(!is_associative_multi_v<boost::container::flat_set<undef>>);
  ASSERT(!is_associative_multi_v<boost::container::flat_map<undef, undef>>);

  ASSERT(is_associative_multi_v<boost::container::flat_multiset<undef>>);
  ASSERT(is_associative_multi_v<boost::container::flat_multimap<undef, undef>>);

  check_range_type<const boost::container::static_vector<undef, 10>, const undef&, std::random_access_iterator_tag, true,
                   range_type_t::sequence>();

  using need_ref = boost::bimaps::relation::mutant_relation<
        boost::bimaps::tags::tagged<const undef, boost::bimaps::relation::member_at::left>,
        boost::bimaps::tags::tagged<const undef, boost::bimaps::relation::member_at::right>, mpl_::na, false>;

  check_range_type<boost::bimaps::bimap<undef, undef>, need_ref&, std::bidirectional_iterator_tag, false,
                   range_type_t::set_like>();

  static_assert(!is_range_v<boost::lockfree::queue<undef>>);

  check_range_type<boost::circular_buffer<undef>, undef&, std::random_access_iterator_tag, false,
                   range_type_t::queue_like>();
*/
}

int main() {
  test_example_graph_representations();
  test_type_traits();
}