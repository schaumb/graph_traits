//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <bxlx/classify/classify.hpp>

#include <array>
#include <type_traits>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <forward_list>
#include <bitset>
#include <string>
#include <optional>
#include <functional>
#include <atomic>
#include <memory>

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

using namespace bxlx::graph::classification;

using type_classification = type;

static_assert(classify<std::array<int, 10>> == type_classification::range);
static_assert(classify<int[10]> == type_classification::range);
static_assert(classify<std::bitset<10>> == type_classification::bitset);
static_assert(classify<const std::bitset<10>> == type_classification::bitset);
static_assert(classify<std::vector<bool>> == type_classification::bitset);
static_assert(classify<std::string> == type_classification::indeterminate);
static_assert(classify<std::wstring> == type_classification::indeterminate);
static_assert(classify<std::basic_string<decltype(u8'\0')>> == type_classification::indeterminate);
static_assert(classify<std::string_view> == type_classification::indeterminate);
static_assert(classify<std::wstring_view> == type_classification::indeterminate);
static_assert(classify<std::basic_string_view<decltype(u8'\0')>> == type_classification::indeterminate);
static_assert(classify<char> == type_classification::indeterminate);

static_assert(classify<std::vector<int>> == type_classification::range);
static_assert(classify<std::deque<bool>> == type_classification::range);
static_assert(classify<std::map<int, int>> == type_classification::map_like);
static_assert(classify<std::multimap<int, int>> == type_classification::map_like);
static_assert(classify<std::unordered_map<int, int>> == type_classification::map_like);
static_assert(classify<std::unordered_multimap<int, int>> == type_classification::map_like);
static_assert(classify<std::set<int, int>> == type_classification::range);
static_assert(classify<std::multiset<int, int>> == type_classification::range);
static_assert(classify<std::unordered_set<int, int>> == type_classification::range);
static_assert(classify<std::unordered_multiset<int, int>> == type_classification::range);
static_assert(classify<std::list<int>> == type_classification::range);
static_assert(classify<std::forward_list<int>> == type_classification::range);

static_assert(classify<std::pair<int, int>> == type_classification::tuple_like);
static_assert(classify<std::tuple<int, int>> == type_classification::tuple_like);
static_assert(classify<std::tuple<bool, int, int>> == type_classification::tuple_like);
static_assert(classify<std::optional<int>> == type_classification::optional);
static_assert(classify<int*> == type_classification::optional);
static_assert(classify<bool> == type_classification::bool_t);
static_assert(classify<std::vector<bool>::reference> == type_classification::bool_t);
static_assert(classify<std::bitset<10>::reference> == type_classification::bool_t);
static_assert(classify<int> == type_classification::index);
static_assert(classify<std::ptrdiff_t> == type_classification::index);
static_assert(classify<std::size_t> == type_classification::index);
static_assert(classify<unsigned char> == type_classification::index);

static_assert(classify<std::atomic<std::size_t>> == type_classification::index);
static_assert(classify<std::atomic<int>> == type_classification::index);
static_assert(classify<std::atomic<char>> == type_classification::index);

static_assert(classify<std::atomic_flag> == type_classification::indeterminate);
static_assert(classify<std::atomic<bool>> == type_classification::bool_t);

static_assert(classify<std::reference_wrapper<std::size_t>> == type_classification::indeterminate);
static_assert(classify<std::reference_wrapper<const std::size_t>> == type_classification::indeterminate);
static_assert(classify<std::reference_wrapper<int>> == type_classification::indeterminate);
static_assert(classify<std::reference_wrapper<bool>> == type_classification::indeterminate);
static_assert(classify<std::reference_wrapper<char>> == type_classification::indeterminate);

#if defined(__cpp_lib_atomic_ref)
static_assert(classify<std::atomic_ref<bool>> == type_classification::indeterminate);
static_assert(classify<std::atomic_ref<int>> == type_classification::indeterminate);
static_assert(classify<std::atomic_ref<std::size_t>> == type_classification::indeterminate);
#endif

// predeclared classes
class A;
static_assert(classify<A[10]> == type_classification::range);
static_assert(classify<std::vector<A>> == type_classification::range);
static_assert(classify<std::deque<A>> == type_classification::range);
static_assert(classify<std::set<A>> == type_classification::range);
static_assert(classify<std::set<std::pair<A, A>>> == type_classification::range);
static_assert(classify<std::list<A>> == type_classification::range);
static_assert(classify<std::forward_list<A>> == type_classification::range);
static_assert(classify<std::map<A, class B>> == type_classification::map_like);
static_assert(classify<std::map<std::pair<A, A>, class B>> == type_classification::map_like);

static_assert(classify<std::tuple<A, A>> == type_classification::tuple_like);
static_assert(classify<std::pair<A, A>> == type_classification::tuple_like);
static_assert(classify<A*> == type_classification::optional);
static_assert(classify<std::array<A, 10>> == type_classification::range);
static_assert(classify<std::optional<A>> == type_classification::optional);
static_assert(classify<std::unique_ptr<A>> == type_classification::optional);
static_assert(classify<std::unique_ptr<A[]>> == type_classification::indeterminate); // no * operator
static_assert(classify<std::shared_ptr<A>> == type_classification::optional);
// static_assert(classify<std::shared_ptr<A[]>> == type_classification::optional); C++17 unspecified
static_assert(classify<std::optional<std::pair<A, A>>> == type_classification::optional);
static_assert(classify<std::optional<std::optional<A>>> == type_classification::optional);
static_assert(classify<std::array<std::optional<A>, 10>> == type_classification::range);
static_assert(bxlx::graph::type_traits::detail::is_range_v<std::optional<A>[10]>);
static_assert(classify<std::optional<A>[10]> == type_classification::range);
static_assert(classify<std::map<A, std::pair<A, A>>> == type_classification::map_like);

static_assert(classify<std::pair<std::array<A, 10>, std::array<A, 10>>> == type_classification::tuple_like);
static_assert(classify<std::pair<std::optional<A>, std::optional<A>>> == type_classification::tuple_like);
static_assert(classify<std::pair<std::pair<A, A>, std::pair<A, A>>> == type_classification::tuple_like);

static_assert(classify<A> == type_classification::pre_declared);

struct C {};
static_assert(classify<C> == type_classification::indeterminate);

template<class>
struct CICA {};
static_assert(classify<CICA<int>> == type_classification::indeterminate);

template<class, std::size_t>
struct ArrayLike {};
static_assert(classify<ArrayLike<int, 1>> == type_classification::indeterminate);

struct my_index {
    my_index(std::size_t) noexcept {}
    operator std::size_t() noexcept { return 0; }
};

static_assert(classify<my_index> == type_classification::index);


template<class T, std::size_t C>
struct MyArray { // we can guess the size() from tuple_size_v<>
    T t[C];

    template<std::size_t I>
    const std::enable_if_t<(I < C), T>& get() const { return t[I]; }
    [[nodiscard]] T* begin() const { return {}; }
    [[nodiscard]] T* end() const { return {}; }
};

struct MyArray2 {
    int t[10];

    template<std::size_t I>
    const std::enable_if_t<(I < 10), int>& get() const { return t[I]; }

    [[nodiscard]] std::size_t size() const {
        return 10;
    }
    [[nodiscard]] int* begin() const { return {}; }
    [[nodiscard]] int* end() const { return {}; }
};

template<class T>
struct MyTuple {
    T t[10];

    template<std::size_t I>
    const std::enable_if_t<(I < 10), T>& get() const { return t[I]; }
};

struct CTBitset {
    struct reference {
        operator bool() noexcept { return false; }
    };
    [[nodiscard]] constexpr std::size_t size() const { return 5; }
    [[nodiscard]] reference operator[](std::size_t ) { return {}; }
};

static_assert(classify<CTBitset> == type_classification::bitset);

struct Bitset {
    struct reference {
        operator bool() noexcept { return false; }
    };
    [[nodiscard]] std::size_t size() const { return 5; }
    [[nodiscard]] reference operator[](std::size_t ) { return {}; }
};

static_assert(classify<Bitset> == type_classification::bitset);

struct MyString {
    struct my_iterator {
        wchar_t operator*() { return {}; }
        my_iterator& operator++() { return *this; }
        bool operator!=(const my_iterator& oth) const { return false; }
    };
    [[nodiscard]] my_iterator begin() const { return {}; }
    [[nodiscard]] my_iterator end() const { return {}; }
    std::size_t length() const { return {}; }
};
static_assert(classify<MyString> == type_classification::indeterminate);

struct MyRar {
    struct my_iterator {
        int operator*() { return {}; }
        my_iterator& operator++() { return *this; }
        bool operator!=(const my_iterator& oth) const { return false; }
    };
    int operator[](std::size_t) const { return {}; }
    [[nodiscard]] my_iterator begin() const { return {}; }
    [[nodiscard]] my_iterator end() const { return {}; }
    [[nodiscard]] std::size_t size() const { return {}; }
};

static_assert(classify<MyRar> == type_classification::range);

struct MyMap {
    struct Key {};
    struct my_iterator {
        std::pair<Key, int> operator*() { return {}; }
        my_iterator& operator++() { return *this; }
        bool operator!=(const my_iterator& oth) const { return false; }
    };
    [[nodiscard]] std::pair<my_iterator, my_iterator> equal_range(Key const&) const { return {}; }
    [[nodiscard]] my_iterator begin() const { return {}; }
    [[nodiscard]] my_iterator end() const { return {}; }
    [[nodiscard]] std::size_t size() const { return {}; }
};
// static_assert(classify<MyMap> == type_classification::map_like);

struct MySet {
    struct Key {};
    struct my_iterator {
        Key operator*() { return {}; }
        my_iterator& operator++() { return *this; }
        bool operator!=(const my_iterator& oth) const { return false; }
    };
    [[nodiscard]] my_iterator find(Key) const { return {}; }
    [[nodiscard]] my_iterator begin() const { return {}; }
    [[nodiscard]] my_iterator end() const { return {}; }
    [[nodiscard]] std::size_t size() const { return {}; }
};
static_assert(classify<MySet> == type_classification::range);

struct MySizedRange {
    struct Key {};
    struct my_iterator {
        Key operator*() { return {}; }
        my_iterator& operator++() { return *this; }
        bool operator!=(const my_iterator& oth) const { return false; }
    };
    [[nodiscard]] my_iterator begin() const { return {}; }
    [[nodiscard]] my_iterator end() const { return {}; }
    [[nodiscard]] std::size_t size() const { return {}; }
};
static_assert(classify<MySizedRange> == type_classification::range);

struct MyRange {
    struct Key {};
    struct my_iterator {
        Key operator*() { return {}; }
        my_iterator& operator++() { return *this; }
        bool operator!=(const my_iterator& oth) const { return false; }
    };
    [[nodiscard]] my_iterator begin() const { return {}; }
    [[nodiscard]] my_iterator end() const { return {}; }
};
static_assert(classify<MyRange> == type_classification::range);

struct MyOptional {
    struct A ;
    explicit operator bool() const {
        return {};
    }
    A operator*() const;
};

static_assert(classify<MyOptional> == type_classification::optional);

template<class T, class = std::enable_if_t<!std::is_pointer_v<T>>>
struct MyOptional2 {
    explicit operator bool() const {
        return {};
    }
    T& operator*() const;
};

static_assert(classify<MyOptional2<class A>> == type_classification::optional);


template<std::size_t I, class T, std::size_t M>
struct std::tuple_element<I, MyArray<T, M>> {
    using type = std::conditional_t<(I < M), const T&, void>;
};

template<class T, std::size_t M>
struct std::tuple_size<MyArray<T, M>> : std::integral_constant<std::size_t, M> {};

template<std::size_t I>
struct std::tuple_element<I, MyArray2> {
    using type = std::conditional_t<(I < 10), const int&, void>;
};

template<>
struct std::tuple_size<MyArray2> : std::integral_constant<std::size_t, 10> {};

template<std::size_t I, class T>
struct std::tuple_element<I, MyTuple<T>> {
    using type = std::conditional_t<(I < 10), const T&, void>;
};

template<class T>
struct std::tuple_size<MyTuple<T>> : std::integral_constant<std::size_t, 10> {};

static_assert(classify<MyArray<int, 1>> == type_classification::range);
static_assert(classify<MyArray2> == type_classification::range);
static_assert(classify<MyTuple<int>> == type_classification::tuple_like);


static_assert(classify<std::map<int, int, std::less<>>> == type_classification::map_like);
static_assert(classify<std::set<std::pair<int, int>, std::less<>>> == type_classification::range);
static_assert(classify<std::set<std::tuple<int>, std::less<>>> == type_classification::range);


#  define ASSERT(...) static_assert((__VA_ARGS__))

struct edge_prop;
struct node_prop;
struct graph_prop;

using namespace bxlx::graph;
using namespace std;


template <class T>
using templates = typename bxlx::graph::type_traits::detail::template_inspect<T>::types;

template <class Check, bool is_range = false>
constexpr void check_is_tuple() {
    using namespace bxlx::graph::type_traits;
    ASSERT(is_tuple_v<Check>);
    ASSERT(!is_optional_v<Check>);
    ASSERT(is_range == is_range_v<Check>);
    ASSERT(!is_bool_v<Check>);
    ASSERT(!is_index_v<Check>);
    ASSERT(!is_bitset_v<Check>);
}


template <class Check>
constexpr void check_is_optional() {
    using namespace bxlx::graph::type_traits;
    ASSERT(!is_tuple_v<Check>);
    ASSERT(is_optional_v<Check>);
    ASSERT(!is_range_v<Check>);
    ASSERT(!is_bool_v<Check>);
    ASSERT(!is_index_v<Check>);
    ASSERT(!is_bitset_v<Check>);
}

template <class Check>
constexpr void check_is_range() {
    using namespace bxlx::graph::type_traits;
    ASSERT(!is_tuple_v<Check>);
    ASSERT(!is_optional_v<Check>);
    ASSERT(is_range_v<Check>);
    ASSERT(!is_bool_v<Check>);
    ASSERT(!is_index_v<Check>);
    ASSERT(!is_bitset_v<Check>);
}

template <class Check, bool is_range = false>
constexpr void check_is_bitset() {
    using namespace bxlx::graph::type_traits;
    ASSERT(!is_tuple_v<Check>);
    ASSERT(!is_optional_v<Check>);
    ASSERT(is_range == is_range_v<Check>);
    ASSERT(!is_bool_v<Check>);
    ASSERT(!is_index_v<Check>);
    ASSERT(is_bitset_v<Check>);
}


template <class Check, class Reference, class IteratorTag, bool St, bxlx::graph::type_traits::range_type_t Rt>
constexpr void check_range_type() {
    using namespace bxlx::graph::type_traits;
    check_is_range<Check>();
    ASSERT(std::is_same_v<range_reference_t<Check>, Reference>);
    ASSERT(std::is_base_of_v<IteratorTag, range_iterator_tag_t<Check>>);
    ASSERT(St == range_is_continuous_v<Check>);
    ASSERT(Rt == range_type_v<Check>);
}



constexpr bool test_type_traits() {
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
    return true;
}

constexpr static bool R = test_type_traits();