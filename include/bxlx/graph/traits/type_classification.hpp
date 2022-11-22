//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef GRAPH_TYPE_CLASSIFICATION_HPP
#define GRAPH_TYPE_CLASSIFICATION_HPP

#include <utility>
#include <iterator>
#include <tuple>

namespace bxlx::detail2 {
    template<class T>
    constexpr inline bool is_char_v = std::is_same_v<T, char> || std::is_same_v<T, char16_t> ||
        std::is_same_v<T, char32_t> || std::is_same_v<T, wchar_t> || std::is_same_v<T, decltype(u8'\0')>;
        // C++17 -> u8'\0' is same type as char, but C++20 it is char8_t, which is different from char


    template<class T>
    constexpr inline bool is_index_v = std::is_integral_v<T> && !std::is_same_v<bool, T> && !is_char_v<T>;


    template<class, class = void>
    constexpr inline bool is_tuple_like_v = false;
    template<class T>
    constexpr inline bool is_tuple_like_v<T, std::enable_if_t<sizeof(std::tuple_size<T>)>> = std::tuple_size_v<T>;

    template <class T, std::size_t ...Ix>
    constexpr bool tuple_is_same_values(std::index_sequence<Ix...>) {
        return (std::is_same_v<std::tuple_element_t<0, T>, std::tuple_element_t<Ix, T>> && ...);
    }


    template<class T, bool = is_tuple_like_v<T>, class = void>
    struct optional_traits_impl {};

    template<class T>
    struct optional_traits_impl<T, false, std::void_t<          // not accept tuple like classes
        decltype(static_cast<bool>(std::declval<T&>())),        // can cast to bool == std::is_convertible_v<T, bool>
        decltype(*std::declval<T&>())                           // has operator*()
    >> {
        using value_type = std::remove_cv_t<std::remove_reference_t<decltype(*std::declval<T&>())>>;
    };

    template<class T, class = void>
    struct optional_traits : optional_traits_impl<T> {};

    // reflect on optional to work std::optional with predeclared classes
    template<template <class> class optional_like_t, class real_value_type>
    struct optional_traits<optional_like_t<real_value_type>, std::enable_if_t<std::is_same_v<void *,
        typename optional_traits_impl<optional_like_t<void *>>::value_type
    >>> {
        using value_type = real_value_type;
    };

    template<class T, class = void>
    constexpr inline bool is_optional_v = false;
    template<class T>
    constexpr inline bool is_optional_v<T, std::void_t<typename optional_traits<T>::value_type>>
        = !std::is_void_v<typename optional_traits<T>::value_type>;


    template<class T, std::size_t = sizeof(T)>
    constexpr std::true_type defined_type(int) { return {}; }
    template<class>
    constexpr std::false_type defined_type(...);

    template<class T, class = void>
    struct is_defined : decltype(defined_type<T>(0)) {
    };

    template<class T>
    struct is_defined<T, std::enable_if_t<is_tuple_like_v<T>>> {
        template<std::size_t ...Ix>
        static constexpr bool is_all_defined(std::index_sequence<Ix...>) {
            return (is_defined<std::tuple_element_t<Ix, T>>::value && ...);
        }
        constexpr static bool value = is_all_defined(std::make_index_sequence<std::tuple_size_v<T>>{});
    };

    template<class T>
    struct is_defined<T, std::enable_if_t<is_optional_v<T>>> : is_defined<typename optional_traits<T>::value_type> {};

    template<class T>
    constexpr inline bool is_defined_v = is_defined<T>::value;


    template<class T, bool = is_defined_v<T>>
    constexpr inline bool is_bool_ref_v = false;
    template<class T>
    constexpr inline bool is_bool_ref_v<T, true> =  // type must be defined
        std::is_class_v<T> &&                       // bool ref can be only classes, whose
        std::is_convertible_v<T, bool> &&           // can convert to bool
        !std::is_constructible_v<T, bool>;          // cannot construct from bool

    template<class T>
    constexpr inline bool is_bool_v = std::is_same_v<T, bool> || is_bool_ref_v<T>;


    template<class It, class Sentinel, class = void>
    constexpr inline bool is_iterator_pair_v = false;
    template<class It, class Sentinel>
    constexpr inline bool is_iterator_pair_v<It, Sentinel, std::enable_if_t<
        std::is_convertible_v<decltype(std::declval<It&>() != std::declval<Sentinel&>()), bool> &&
        !std::is_void_v<decltype(*std::declval<It&>())> &&
        std::is_same_v<decltype(++std::declval<It&>()), It&>
    >> = true;

    template<class It, class Sentinel, class = void>
    constexpr inline bool is_random_access_iterator_pair_v = false;
    template<class It, class Sentinel>
    constexpr inline bool is_random_access_iterator_pair_v<It, Sentinel, std::enable_if_t<
        is_iterator_pair_v<It, Sentinel> &&
        std::is_invocable_v<std::minus<>, It, It>
    >> = true;


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

    template<class T, class = void>
    struct range_tuple_traits2 {
        constexpr static bool is_sized = false; //dunno
        constexpr static bool random_access = false;
    };

    template<class T>
    struct range_tuple_traits2<T, std::enable_if_t<is_defined_v<std::tuple_element_t<0, T>>>>
        : range_traits_impl<T>
    {
    };

    template<class T, bool = tuple_is_same_values<T>(std::make_index_sequence<std::tuple_size_v<T>>{})>
    struct range_tuple_traits {
        constexpr static bool is_sized = false; //dunno
        constexpr static bool random_access = false;
    };

    template<class T>
    struct range_tuple_traits<T, true> : range_tuple_traits2<T> {};

    template<class T, bool = is_tuple_like_v<T>, class = void>
    struct range_traits : range_traits_impl<T> {
    };

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
        using value_type = std::tuple_element_t<0, container<the_value_type, Ix>>;
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
    constexpr inline type_classification classify = is_defined_v<T> ? type_classification::indeterminate : type_classification::pre_declared;


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
}

#endif //GRAPH_TYPE_CLASSIFICATION_HPP
