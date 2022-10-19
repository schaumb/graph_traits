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
    struct optional_traits {};

    template<class T>
    struct optional_traits<T*, void> {
        using value_type = typename std::iterator_traits<T*>::value_type;
    };

    template<template <class> class OptionalLike, class real_value_type>
    struct optional_traits<OptionalLike<real_value_type>, void> {
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

    template<class T, bool = is_tuple_like_v<T>, class = void>
    struct range_traits_impl {
    };


    template<class T, bool = is_tuple_like_v<T>, class = void>
    struct range_traits : range_traits_impl<T> {
        constexpr static bool is_sized = false; //dunno
        constexpr static bool random_access = false;
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

    template<class T>
    struct range_traits_impl<T, false, std::enable_if_t<
        !std::is_void_v<typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::reference> &&
        !std::is_void_v<typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::value_type> &&
        std::is_void_v<std::void_t<decltype(std::end(std::declval<T&>()))>>>> {
        using reference = typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::reference;
        using value_type = typename std::iterator_traits<decltype(std::begin(std::declval<T&>()))>::value_type;
    };


    template<class T, bool = is_tuple_like_v<T> && !range_traits<T>::random_access, class = void>
    constexpr inline bool is_range_v = false;
    template<class T>
    constexpr inline bool is_range_v<T, false, std::void_t<
        typename range_traits<T>::reference,
        typename range_traits<T>::value_type
    >> = true;

    template<class T, bool = is_range_v<T>, bool = range_traits<T>::is_sized, class = void>
    constexpr inline bool is_sized_range_v = false;
    template<class T>
    constexpr inline bool is_sized_range_v<T, true, true, void> = true;
    template<class T>
    constexpr inline bool is_sized_range_v<T, true, false, std::void_t<decltype(std::size(std::declval<T>()))>>
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



    template<class T, bool = std::is_class_v<T> && !is_tuple_like_v<T> && !is_optional_v<T> && decltype(defined_type<T>(0))::value >
    constexpr inline bool is_bool_ref_v = false;
    template<class T>
    constexpr inline bool is_bool_ref_v<T, true>
        = std::is_convertible_v<T, bool> && !std::is_constructible_v<T, bool>;

    template<class T, bool = std::is_class_v<T> && !is_tuple_like_v<T> && !is_optional_v<T>>
    constexpr inline bool is_bool_v = std::is_same_v<T, bool>;
    template<class T>
    constexpr inline bool is_bool_v<T, true> = is_bool_ref_v<T>;

    template<class T, bool = std::is_array_v<T> && (std::extent_v<T> != 0) || range_traits<T>::random_access, bool = is_sized_range_v<T>, class = void>
    constexpr inline bool is_random_access_range_v = false;
    template<class T>
    constexpr inline bool is_random_access_range_v<T, true, true, void> = true;
    template<class T>
    constexpr inline bool is_random_access_range_v<T, false, true, std::void_t<
        decltype(std::declval<T&>()[std::size_t{}])
    >> = std::is_same_v<
        std::remove_cv_t<std::remove_reference_t<decltype(std::declval<T&>()[std::size_t{}])>>,
        std::remove_cv_t<std::remove_reference_t<decltype(*std::begin(std::declval<T&>()))>>>;


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
        compile_time_random_access_range, // array, U[X]
        compile_time_bitset_like_container, // bitset<>
        bitset_like_container, // vector<bool>
        string_like_range, // string, basic_string<any_char>
        random_access_range, // vector, deque
        map_like_container, // map, multimap, unordered_map, unordered_multimap, flat_map, flat_multimap
        set_like_container, // set, multiset, unordered_set, unordered_multiset, flat_set, flat_multiset
        sized_range, // list
        range, // forward_list
        tuple_like, // pair, tuple
        optional, // optional, U*
        bool_t, // bool, vector<bool>::reference, bitset<>::reference
        index, // size_t, ptrdiff_t, int, ...
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
    constexpr inline type_classification classify<T, std::enable_if_t<is_range_v<T> && !is_sized_range_v<T>>>
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

    static_assert(classify<A> == type_classification::pre_declared);

    struct C {};
    static_assert(classify<class C> == type_classification::indeterminate);
}

#endif //GRAPH_TYPE_CLASSIFICATION_HPP
