//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef GRAPH_TYPE_CLASSIFICATION_HPP
#define GRAPH_TYPE_CLASSIFICATION_HPP

#include <tuple>
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

namespace bxlx::detail2 {
    template<class T, class = void>
    struct optional_traits2 {
    };

    template<class T>
    struct optional_traits2<T, std::void_t<
        decltype(static_cast<bool>(std::declval<T&>())),
        decltype(*std::declval<T&>())
    >> {
        using value_type = decltype(*std::declval<T&>());
    };

    template<class T, class = void>
    struct optional_traits : optional_traits2<T> {};

    template<class T>
    struct optional_traits<T*, void> {
        using value_type = typename std::iterator_traits<T*>::value_type;
    };

    template<template <class> class OptionalLike, class real_value_type>
    struct optional_traits<OptionalLike<real_value_type>, std::void_t<
        std::enable_if_t<std::is_same_v<void *, std::remove_cv_t<std::remove_reference_t<typename optional_traits2<OptionalLike<void *>>::value_type>>>>
    >> {
        using value_type = real_value_type;
    };

    template<class T, size_t = sizeof(T)>
    constexpr std::true_type defined_type(int) { return {}; }
    template<class T>
    constexpr std::false_type defined_type(...);


    template<class T, class = void>
    constexpr inline bool is_tuple_like_v = false;
    template<class T>
    constexpr inline bool is_tuple_like_v<T, std::enable_if_t<sizeof(std::tuple_size<T>)>> = std::tuple_size_v<T>;

    template<class T>
    constexpr inline bool is_char_v = std::is_same_v<T, char> || std::is_same_v<T, char16_t> ||
                                      std::is_same_v<T, char32_t> || std::is_same_v<T, wchar_t> || std::is_same_v<T, decltype(u8'\0')>;

    template<class T>
    constexpr inline bool is_index_v = std::is_integral_v<T> && !std::is_same_v<bool, T> && !is_char_v<T>;

    template <class T, std::size_t ...Ix>
    constexpr auto tuple_is_same_values(std::index_sequence<Ix...>) {
        using V = std::tuple_element_t<0, T>;
        return std::bool_constant<(std::is_same_v<V, std::tuple_element_t<Ix, T>> && ...)>{};
    }

    template <class T>
    constexpr auto tuple_is_same_values() {
        return tuple_is_same_values<T>(std::make_index_sequence<std::tuple_size_v<T>>{});
    }

    template<class T, class = void>
    struct range_traits_impl2 {
        constexpr static bool random_access = false;
    };

    template<class T>
    struct range_traits_impl2<T, std::void_t<decltype(*std::begin(std::declval<T&>()))>>{
        using reference = decltype(*std::begin(std::declval<T&>()));
        using value_type = std::remove_reference_t<reference>;
        constexpr static bool random_access = std::is_invocable_v<std::minus<>, decltype(std::begin(std::declval<T&>())), decltype(std::begin(std::declval<T&>()))>;
    };

    template<class T, class = void>
    struct range_traits_impl1 : range_traits_impl2<T> {
    };

    template<class T>
    struct range_traits_impl1<T, std::enable_if_t<
        !std::is_void_v<typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::reference> &&
        !std::is_void_v<typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::value_type> &&
        std::is_void_v<std::void_t<decltype(std::end(std::declval<T&>()))>>>> {
        using reference = typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::reference;
        using value_type = typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::value_type;

        constexpr static bool random_access =
            std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::iterator_category>;
    };

    template<class T, class = void>
    struct range_traits_impl : range_traits_impl1<T> {
        constexpr static bool is_sized = false; //dunno
    };

    template<class T>
    struct range_traits_impl<T, std::void_t<decltype(std::size(std::declval<T&>()))>> : range_traits_impl1<T> {
        constexpr static bool is_sized = true;
    };

    template<class T, bool = is_tuple_like_v<T>, class = void>
    struct range_traits : range_traits_impl<T> {
    };

    template<class T, class = void>
    struct range_tuple_traits2 {
        constexpr static bool is_sized = false; //dunno
        constexpr static bool random_access = false;
    };

    template<class T, bool = decltype(tuple_is_same_values<T>())::value>
    struct range_tuple_traits {
        constexpr static bool is_sized = false; //dunno
        constexpr static bool random_access = false;
    };

    template<class T>
    struct range_tuple_traits<T, true> : range_tuple_traits2<T> {};

    template<class T>
    struct range_traits<T, true, void> : range_tuple_traits<T> {
    };

    template<class the_value_type, std::size_t Ix>
    struct range_traits<the_value_type [Ix], false, void> {
        using value_type = the_value_type;
        using reference = value_type&;
        constexpr static bool is_sized = true;
        constexpr static bool random_access = true;
    };


    template<class T>
    struct range_traits<T, false, std::enable_if_t<std::is_void_v<std::void_t<typename optional_traits<T>::value_type>>>> {
        constexpr static bool is_sized = false; // dunno
        constexpr static bool random_access = false;
    };

    template<template <class, std::size_t> class container, class the_value_type, std::size_t Ix>
    struct range_traits<container<the_value_type, Ix>, true, void> {
        using value_type = the_value_type;
        using reference = value_type&;
        constexpr static bool is_sized = true;
        constexpr static bool random_access = true;
    };


    template<class T, bool = is_tuple_like_v<T> && !range_traits<T>::random_access, class = void>
    constexpr inline bool is_range_v = range_traits<T>::random_access;
    template<class T>
    constexpr inline bool is_range_v<T, false, std::void_t<
        typename range_traits<T>::reference,
        typename range_traits<T>::value_type
    >> = true;

    template<class T, bool = is_range_v<T> && !range_traits<T>::is_sized, class = void>
    constexpr inline bool is_sized_range_v = range_traits<T>::is_sized;
    template<class T>
    constexpr inline bool is_sized_range_v<T, true, std::void_t<decltype(std::size(std::declval<T>()))>>
        = is_index_v<decltype(std::size(std::declval<T>()))>;

    template<class T, bool = is_sized_range_v<T> && !range_traits<T>::random_access, class = void>
    constexpr inline bool is_map_like_container_v = false;
    template<class T>
    constexpr inline bool is_map_like_container_v<T, true, std::void_t<
        decltype(std::declval<T>().find(std::declval< std::tuple_element_t<0, typename range_traits<T>::value_type> >() )),
        decltype(std::tuple_size_v<typename range_traits<T>::value_type> == 2)
    >> = std::is_same_v<decltype(std::declval<T>().find(std::declval< std::tuple_element_t<0, typename range_traits<T>::value_type> >())),
        decltype(std::begin(std::declval<T&>()))> &&
        !std::is_void_v<std::tuple_element_t<1, typename range_traits<T>::value_type>>;

    template<class T, bool = is_sized_range_v<T> && !is_map_like_container_v<T> && !range_traits<T>::random_access, class = void>
    constexpr inline bool is_set_like_container_v = false;
    template<class T>
    constexpr inline bool is_set_like_container_v<T, true, std::void_t<
        decltype(std::declval<T>().find(std::declval<typename range_traits<T>::reference>()))
    >> = std::is_same_v<decltype(std::declval<T>().find(*std::begin(std::declval<T&>()))),
        decltype(std::begin(std::declval<T&>()))>;



    template<class T, bool = !is_map_like_container_v<T> && !is_tuple_like_v<T>, class = void>
    constexpr inline bool is_optional_v = false;
    template<class T>
    constexpr inline bool is_optional_v<T, true, std::void_t<typename optional_traits<T>::value_type>>
        = !std::is_void_v<typename optional_traits<T>::value_type>;



    template<class T, class = void>
    struct is_defined : decltype(defined_type<T>(0)) {
    };

    template<class T>
    struct is_defined<T, std::enable_if_t<
        is_tuple_like_v<T>
    >> {
        template<std::size_t ...Ix>
        constexpr static auto is_all_defined(std::index_sequence<Ix...>) {
            return std::bool_constant<(is_defined<std::tuple_element_t<Ix, T>>::value && ...)>{};
        }
        constexpr static bool value = decltype(is_all_defined(std::make_index_sequence<std::tuple_size_v<T>>{}))::value;
    };

    template<class T>
    struct is_defined<T, std::enable_if_t<is_optional_v<T>>> : is_defined<typename optional_traits<T>::value_type> {};


    template<class T>
    struct range_tuple_traits2<T, std::enable_if_t<is_defined<std::tuple_element_t<0, T>>::value>>
        : range_traits_impl<T>
    {
    };

    template<class T, bool = std::is_class_v<T> && !is_tuple_like_v<T> && !is_optional_v<T> && decltype(defined_type<T>(0))::value >
    constexpr inline bool is_bool_ref_v = false;
    template<class T>
    constexpr inline bool is_bool_ref_v<T, true>
        = std::is_convertible_v<T, bool> && !std::is_constructible_v<T, bool>;

    template<class T, bool = std::is_class_v<T> && !is_tuple_like_v<T> && !is_optional_v<T>>
    constexpr inline bool is_bool_v = std::is_same_v<T, bool>;
    template<class T>
    constexpr inline bool is_bool_v<T, true> = is_bool_ref_v<T>;


    template<class T, bool = is_range_v<T>>
    constexpr inline bool is_string_like_v = false;
    template<class T>
    constexpr inline bool is_string_like_v<T, true>
        = is_char_v<std::remove_cv_t<std::remove_reference_t<typename range_traits<T>::reference>>>;

    template<class T, bool = !is_tuple_like_v<T> && !is_optional_v<T> && !is_string_like_v<T>, class = void>
    constexpr inline bool is_bitset_like_v = false;
    template<class T>
    constexpr inline bool is_bitset_like_v<T, true, std::void_t<
        decltype(std::declval<T>()[std::size(std::declval<T>())])
    >> = is_index_v<decltype(std::size(std::declval<T>()))> &&
         is_bool_ref_v<decltype(std::declval<T>()[std::size(std::declval<T>())])>;

    template<class T, bool = (std::is_array_v<T> && std::extent_v<T> != 0) || range_traits<T>::random_access, bool = is_sized_range_v<T> && !is_bitset_like_v<T>, class = void>
    constexpr inline bool is_random_access_range_v = false;
    template<class T>
    constexpr inline bool is_random_access_range_v<T, true, true, void> = true;
    template<class T>
    constexpr inline bool is_random_access_range_v<T, false, true, std::void_t<
        decltype(std::declval<T&>()[std::size_t{}])
    >> = std::is_same_v<
        std::remove_cv_t<std::remove_reference_t<decltype(std::declval<T&>()[std::size_t{}])>>,
        std::remove_cv_t<std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>>>;


    template<class T, bool = std::is_array_v<T> && std::extent_v<T> != 0, bool = is_tuple_like_v<T>, bool = is_bitset_like_v<T>, class = void>
    constexpr inline std::size_t is_compile_time_sized_v = 0;

    template<class T>
    constexpr inline std::size_t is_compile_time_sized_v<T, true, false, false> = std::extent_v<T>;
    template<class T>
    constexpr inline std::size_t is_compile_time_sized_v<T, false, true, false> = std::tuple_size_v<T>;
    template<class T>
    constexpr inline std::size_t is_compile_time_sized_v<T, false, false, true, std::void_t<
        std::integral_constant<std::size_t, std::size(T{})>
    >> = std::size(T{}) > 0;

    template<class T>
    constexpr inline std::size_t is_compile_time_bitset_like_v = is_bitset_like_v<T> ? is_compile_time_sized_v<T> : 0;

    template<class T>
    constexpr inline std::size_t is_compile_time_random_access_range_v = is_random_access_range_v<T> ? is_compile_time_sized_v<T> : 0;


    enum class type_classification {
        indeterminate,
        pre_declared,
        compile_time_random_access_range,
        compile_time_bitset_like_container,
        bitset_like_container,
        string_like_range,
        random_access_range,
        map_like_container,
        set_like_container,
        sized_range,
        range,
        tuple_like,
        optional,
        bool_t,
        index,
    };

    template<class T, class = void>
    constexpr inline type_classification classify = decltype(defined_type<T&>(0))::value ? type_classification::indeterminate : type_classification::pre_declared;


    template<class T>
    constexpr inline type_classification classify<T,
        std::enable_if_t<is_tuple_like_v<T> && !is_range_v<T>>>
        = type_classification::tuple_like;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_compile_time_random_access_range_v<T> != 0>>
        = type_classification::compile_time_random_access_range;
    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_compile_time_bitset_like_v<T>>>
        = type_classification::compile_time_bitset_like_container;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<!is_compile_time_bitset_like_v<T> &&
        is_bitset_like_v<T>>>
        = type_classification::bitset_like_container;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_string_like_v<T>>>
        = type_classification::string_like_range;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<!is_compile_time_random_access_range_v<T> &&
        !is_string_like_v<T> && !is_bitset_like_v<T> && is_random_access_range_v<T>>>
        = type_classification::random_access_range;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_map_like_container_v<T>>>
        = type_classification::map_like_container;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_set_like_container_v<T>>>
        = type_classification::set_like_container;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_sized_range_v<T> && !is_map_like_container_v<T>
        && !is_set_like_container_v<T> && !is_random_access_range_v<T> && !is_bitset_like_v<T>>>
        = type_classification::sized_range;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_range_v<T> && !is_sized_range_v<T> && !is_string_like_v<T>>>
        = type_classification::range;

    template<class T>
    constexpr inline type_classification classify<T,
        std::enable_if_t<is_optional_v<T> && !std::is_array_v<T>>>
        = type_classification::optional;

    template<class T>
    constexpr inline type_classification classify<T,
        std::enable_if_t<is_bool_v<T>>>
        = type_classification::bool_t;

    template<class T>
    constexpr inline type_classification classify<T,
        std::enable_if_t<is_index_v<T>>>
        = type_classification::index;


    static_assert(!std::is_void_v<typename range_traits<std::array<int, 10>>::value_type>);
    static_assert(classify<std::array<int, 10>> == type_classification::compile_time_random_access_range);
    static_assert(classify<int[10]> == type_classification::compile_time_random_access_range);
    static_assert(classify<std::bitset<10>> == type_classification::compile_time_bitset_like_container);
    static_assert(classify<std::vector<bool>> == type_classification::bitset_like_container);
    static_assert(classify<std::string> == type_classification::string_like_range);
    static_assert(classify<std::wstring> == type_classification::string_like_range);
    static_assert(classify<std::basic_string<decltype(u8'\0')>> == type_classification::string_like_range);

    static_assert(classify<std::vector<int>> == type_classification::random_access_range);
    static_assert(classify<std::deque<bool>> == type_classification::random_access_range);
    static_assert(classify<std::map<int, int>> == type_classification::map_like_container);
    static_assert(classify<std::multimap<int, int>> == type_classification::map_like_container);
    static_assert(classify<std::unordered_map<int, int>> == type_classification::map_like_container);
    static_assert(classify<std::unordered_multimap<int, int>> == type_classification::map_like_container);
    static_assert(classify<std::set<int, int>> == type_classification::set_like_container);
    static_assert(classify<std::multiset<int, int>> == type_classification::set_like_container);
    static_assert(classify<std::unordered_set<int, int>> == type_classification::set_like_container);
    static_assert(classify<std::unordered_multiset<int, int>> == type_classification::set_like_container);
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

    // predeclared classes
    class A;
    static_assert(classify<A[10]> == type_classification::compile_time_random_access_range);
    static_assert(classify<std::vector<A>> == type_classification::random_access_range);
    static_assert(classify<std::deque<A>> == type_classification::random_access_range);
    static_assert(classify<std::set<A>> == type_classification::set_like_container);
    static_assert(classify<std::list<A>> == type_classification::sized_range);
    static_assert(classify<std::forward_list<A>> == type_classification::range);
    static_assert(classify<std::map<A, class B>> == type_classification::map_like_container);

    static_assert(classify<std::tuple<A, A>> == type_classification::tuple_like);
    static_assert(classify<std::pair<A, A>> == type_classification::tuple_like);
    static_assert(classify<A*> == type_classification::optional);
    static_assert(classify<std::array<A, 10>> == type_classification::compile_time_random_access_range);
    static_assert(classify<std::optional<A>> == type_classification::optional);
    static_assert(classify<std::optional<std::pair<A, A>>> == type_classification::optional);
    static_assert(classify<std::optional<std::optional<A>>> == type_classification::optional);
    static_assert(classify<std::array<std::optional<A>, 10>> == type_classification::compile_time_random_access_range);
    static_assert(classify<std::optional<A>[10]> == type_classification::compile_time_random_access_range);
    static_assert(classify<std::map<A, std::pair<A, A>>> == type_classification::map_like_container);

    static_assert(classify<std::pair<std::array<A, 10>, std::array<A, 10>>> == type_classification::tuple_like);
    static_assert(classify<std::pair<std::optional<A>, std::optional<A>>> == type_classification::tuple_like);
    static_assert(classify<std::pair<std::pair<A, A>, std::pair<A, A>>> == type_classification::tuple_like);

    static_assert(classify<A> == type_classification::pre_declared);

    struct C {};
    static_assert(classify<C> == type_classification::indeterminate);

    template<class T>
    struct CICA {};
    static_assert(classify<CICA<int>> == type_classification::indeterminate);

    template<class T, std::size_t C>
    struct ArrayLike {};
    static_assert(classify<ArrayLike<int, 1>> == type_classification::indeterminate);


    template<class T, std::size_t C>
    struct MyArray { // we can guess the size() from template argument
        T t[C];

        template<std::size_t I>
        const std::enable_if_t<(I < C), T>& get() const { return t[I]; }
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
            operator bool() { return false; }
        };
        [[nodiscard]] constexpr std::size_t size() const { return 5; }
        [[nodiscard]] reference operator[](std::size_t ) { return {}; }
    };

    static_assert(classify<CTBitset> == type_classification::compile_time_bitset_like_container);

    struct Bitset {
        struct reference {
            operator bool() { return false; }
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
    };
    static_assert(classify<MyString> == type_classification::string_like_range);

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
    static_assert(classify<MyRar> == type_classification::random_access_range);

    struct MyMap {
        struct Key {};
        struct my_iterator {
            std::pair<Key, int> operator*() { return {}; }
            my_iterator& operator++() { return *this; }
            bool operator!=(const my_iterator& oth) const { return false; }
        };
        [[nodiscard]] my_iterator find(Key) const { return {}; }
        [[nodiscard]] my_iterator begin() const { return {}; }
        [[nodiscard]] my_iterator end() const { return {}; }
        [[nodiscard]] std::size_t size() const { return {}; }
    };
    static_assert(classify<MyMap> == type_classification::map_like_container);

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
    static_assert(classify<MySet> == type_classification::set_like_container);

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
}

template<std::size_t I, class T, std::size_t M>
struct ::std::tuple_element<I, bxlx::detail2::MyArray<T, M>> {
    using type = std::conditional_t<(I < M), const T&, void>;
};

template<class T, std::size_t M>
struct ::std::tuple_size<bxlx::detail2::MyArray<T, M>> : std::integral_constant<std::size_t, M> {};

template<std::size_t I>
struct ::std::tuple_element<I, bxlx::detail2::MyArray2> {
    using type = std::conditional_t<(I < 10), const int&, void>;
};

template<>
struct ::std::tuple_size<bxlx::detail2::MyArray2> : std::integral_constant<std::size_t, 10> {};

template<std::size_t I, class T>
struct ::std::tuple_element<I, bxlx::detail2::MyTuple<T>> {
    using type = std::conditional_t<(I < 10), const T&, void>;
};

template<class T>
struct ::std::tuple_size<bxlx::detail2::MyTuple<T>> : std::integral_constant<std::size_t, 10> {};

static_assert(bxlx::detail2::classify<bxlx::detail2::MyArray<int, 1>> == bxlx::detail2::type_classification::compile_time_random_access_range);
static_assert(bxlx::detail2::classify<bxlx::detail2::MyArray2> == bxlx::detail2::type_classification::compile_time_random_access_range);
static_assert(bxlx::detail2::classify<bxlx::detail2::MyTuple<int>> == bxlx::detail2::type_classification::tuple_like);


#endif //GRAPH_TYPE_CLASSIFICATION_HPP
