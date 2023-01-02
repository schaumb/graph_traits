//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <bxlx/graph/traits/type_classification.hpp>

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

using namespace bxlx::detail2;

static_assert(!std::is_void_v<typename range_traits<std::array<int, 10>>::value_type>);
static_assert(classify<std::array<int, 10>> == type_classification::random_access_range);
static_assert(classify<int[10]> == type_classification::random_access_range);
static_assert(classify<std::bitset<10>> == type_classification::bitset_like_container);
static_assert(classify<const std::bitset<10>> == type_classification::bitset_like_container);
static_assert(classify<std::vector<bool>> == type_classification::bitset_like_container);
static_assert(classify<std::string> == type_classification::indeterminate);
static_assert(classify<std::wstring> == type_classification::indeterminate);
static_assert(classify<std::basic_string<decltype(u8'\0')>> == type_classification::indeterminate);
static_assert(classify<std::string_view> == type_classification::indeterminate);
static_assert(classify<std::wstring_view> == type_classification::indeterminate);
static_assert(classify<std::basic_string_view<decltype(u8'\0')>> == type_classification::indeterminate);
static_assert(classify<char> == type_classification::indeterminate);

static_assert(classify<std::vector<int>> == type_classification::random_access_range);
static_assert(classify<std::deque<bool>> == type_classification::random_access_range);
static_assert(classify<std::map<int, int>> == type_classification::sized_range);
static_assert(classify<std::multimap<int, int>> == type_classification::sized_range);
static_assert(classify<std::unordered_map<int, int>> == type_classification::sized_range);
static_assert(classify<std::unordered_multimap<int, int>> == type_classification::sized_range);
static_assert(classify<std::set<int, int>> == type_classification::sized_range);
static_assert(classify<std::multiset<int, int>> == type_classification::sized_range);
static_assert(classify<std::unordered_set<int, int>> == type_classification::sized_range);
static_assert(classify<std::unordered_multiset<int, int>> == type_classification::sized_range);
static_assert(classify<std::list<int>> == type_classification::sized_range);
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
static_assert(classify<std::atomic<bool>> == type_classification::indeterminate);

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
static_assert(classify<A[10]> == type_classification::random_access_range);
static_assert(classify<std::vector<A>> == type_classification::random_access_range);
static_assert(classify<std::deque<A>> == type_classification::random_access_range);
static_assert(classify<std::set<A>> == type_classification::sized_range);
static_assert(classify<std::set<std::pair<A, A>>> == type_classification::sized_range);
static_assert(classify<std::list<A>> == type_classification::sized_range);
static_assert(classify<std::forward_list<A>> == type_classification::range);
static_assert(classify<std::map<A, class B>> == type_classification::sized_range);
static_assert(classify<std::map<std::pair<A, A>, class B>> == type_classification::sized_range);

static_assert(classify<std::tuple<A, A>> == type_classification::tuple_like);
static_assert(classify<std::pair<A, A>> == type_classification::tuple_like);
static_assert(classify<A*> == type_classification::optional);
static_assert(classify<std::array<A, 10>> == type_classification::random_access_range);
static_assert(classify<std::optional<A>> == type_classification::optional);
static_assert(classify<std::unique_ptr<A>> == type_classification::optional);
static_assert(classify<std::unique_ptr<A[]>> == type_classification::indeterminate); // no * operator
static_assert(classify<std::shared_ptr<A>> == type_classification::optional);
static_assert(classify<std::shared_ptr<A[]>> == type_classification::optional);
static_assert(classify<std::optional<std::pair<A, A>>> == type_classification::optional);
static_assert(classify<std::optional<std::optional<A>>> == type_classification::optional);
static_assert(classify<std::array<std::optional<A>, 10>> == type_classification::random_access_range);
static_assert(classify<std::optional<A>[10]> == type_classification::random_access_range);
static_assert(classify<std::map<A, std::pair<A, A>>> == type_classification::sized_range);

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

static_assert(classify<CTBitset> == type_classification::bitset_like_container);

struct Bitset {
    struct reference {
        operator bool() noexcept { return false; }
    };
    [[nodiscard]] std::size_t size() const { return 5; }
    [[nodiscard]] reference operator[](std::size_t ) { return {}; }
};

static_assert(classify<Bitset> == type_classification::bitset_like_container);

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

static_assert(bxlx::detail2::has_subscript_operator_v<MyRar>);
static_assert(classify<MyRar> == type_classification::random_access_range);

struct MyMap {
    struct Key {};
    struct my_iterator {
        std::pair<Key, int> operator*() { return {}; }
        my_iterator& operator++() { return *this; }
        bool operator!=(const my_iterator& oth) const { return false; }
    };
    [[nodiscard]] std::pair<my_iterator, my_iterator> equal_range(Key) const { return {}; }
    [[nodiscard]] my_iterator begin() const { return {}; }
    [[nodiscard]] my_iterator end() const { return {}; }
    [[nodiscard]] std::size_t size() const { return {}; }
};
static_assert(classify<MyMap> == type_classification::sized_range);

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
static_assert(classify<MySet> == type_classification::sized_range);

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
static_assert(classify<MySizedRange> == type_classification::sized_range);

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
struct ::std::tuple_element<I, MyArray<T, M>> {
    using type = std::conditional_t<(I < M), const T&, void>;
};

template<class T, std::size_t M>
struct ::std::tuple_size<MyArray<T, M>> : std::integral_constant<std::size_t, M> {};

template<std::size_t I>
struct ::std::tuple_element<I, MyArray2> {
    using type = std::conditional_t<(I < 10), const int&, void>;
};

template<>
struct ::std::tuple_size<MyArray2> : std::integral_constant<std::size_t, 10> {};

template<std::size_t I, class T>
struct ::std::tuple_element<I, MyTuple<T>> {
    using type = std::conditional_t<(I < 10), const T&, void>;
};

template<class T>
struct ::std::tuple_size<MyTuple<T>> : std::integral_constant<std::size_t, 10> {};

static_assert(classify<MyArray<int, 1>> == type_classification::random_access_range);
static_assert(classify<MyArray2> == type_classification::random_access_range);
static_assert(classify<MyTuple<int>> == type_classification::tuple_like);


static_assert(classify<std::map<int, int, std::less<>>> == type_classification::sized_range);
static_assert(classify<std::set<std::pair<int, int>, std::less<>>> == type_classification::sized_range);
static_assert(classify<std::set<std::tuple<int>, std::less<>>> == type_classification::sized_range);


static_assert(std::is_same_v<get_begin_iterator_t<const std::vector<int>>, std::vector<int>::const_iterator>);
static_assert(std::is_same_v<range_traits_type<const std::vector<int>>, const int>);

static_assert(std::is_same_v<range_traits_type<const std::array<struct Dimy, 5>>, const struct Dimy>);

static_assert(std::is_same_v<range_traits_type<const std::array<int, 5>>, const int>);
static_assert(std::is_same_v<range_traits_type<std::initializer_list<int>>, const int>);
static_assert(std::is_same_v<range_traits_type<const std::initializer_list<int>>, const int>);
static_assert(std::is_same_v<range_traits_type<const int[6]>, const int>);
static_assert(std::is_same_v<optional_traits_type<const std::optional<int>>, const int>);
static_assert(std::is_same_v<optional_traits_type<const std::optional<struct Dimy>>, const struct Dimy>);
static_assert(std::is_same_v<optional_traits_type<const struct Dimy*>, const struct Dimy>);

static_assert(classify<const bool> == type_classification::bool_t);
static_assert(classify<const std::map<int, int>> == type_classification::sized_range);
static_assert(classify<const std::unordered_map<int, int>> == type_classification::sized_range);

static_assert(bxlx::detail2::is_set_like_container_v<std::set<int>>);
static_assert(bxlx::detail2::is_set_like_container_v<std::set<std::pair<int, int>>>);
static_assert(bxlx::detail2::is_set_like_container_v<std::set<std::pair<const int, int>>>);
static_assert(bxlx::detail2::is_set_like_container_v<std::set<int, std::greater<>>>);
static_assert(bxlx::detail2::is_set_like_container_v<std::multiset<int>>);
static_assert(!bxlx::detail2::is_set_like_container_v<std::map<int, int>>);

static_assert(!bxlx::detail2::is_multi_v<std::set<int>>);
static_assert(!bxlx::detail2::is_multi_v<std::set<int, std::less<>>>);
static_assert(bxlx::detail2::is_multi_v<std::multiset<int>>);
static_assert(bxlx::detail2::is_multi_v<std::multiset<int, std::less<>>>);
static_assert(!bxlx::detail2::is_multi_v<std::map<int, int>>);
static_assert(!bxlx::detail2::is_multi_v<std::map<int, int, std::less<>>>);
static_assert(bxlx::detail2::is_multi_v<std::multimap<int, int>>);
static_assert(bxlx::detail2::is_multi_v<std::multimap<int, int, std::less<>>>);


int main() {}