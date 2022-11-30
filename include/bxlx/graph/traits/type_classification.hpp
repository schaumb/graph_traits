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
    template<class CVT, class T = std::remove_cv_t<CVT>>
    constexpr inline bool is_char_v = std::is_same_v<T, char> || std::is_same_v<T, char16_t> ||
        std::is_same_v<T, char32_t> || std::is_same_v<T, wchar_t> || std::is_same_v<T, decltype(u8'\0')>;
        // C++17 -> u8'\0' is same type as char, but C++20 it is char8_t, which is different from char


    template<class, class = void>
    constexpr inline bool is_tuple_like_v = false;
    template<class T>
    constexpr inline bool is_tuple_like_v<T, std::void_t<decltype(std::tuple_size<T>::value)>> = std::tuple_size_v<T>;

    template<class T, std::size_t ...Ix>
    constexpr bool tuple_has_same_values(std::index_sequence<Ix...>) {
        return (std::is_same_v<std::tuple_element_t<0, T>, std::tuple_element_t<Ix, T>> && ...);
    }

    template<class T, class IS = std::make_index_sequence<std::tuple_size_v<T>>>
    constexpr inline bool tuple_has_same_values_v = tuple_has_same_values<T>(IS{});


    template<class T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    template<class T, class U>
    using copy_reference_t = std::conditional_t<std::is_lvalue_reference_v<T>,
            std::add_lvalue_reference_t<U>,
            std::conditional_t<std::is_rvalue_reference_v<T>,
                std::add_rvalue_reference_t<U>,
                U>>;

    template<class T, class U>
    struct copy_cvref {
        using t_without_ref = std::remove_reference_t<T>;
        using copied_const = std::conditional_t<std::is_const_v<t_without_ref>, std::add_const_t<U>, U>;
        using copied_cv = std::conditional_t<std::is_volatile_v<t_without_ref>,
                                             std::add_volatile_t<copied_const>, copied_const>;
        using type = copy_reference_t<T, copied_cv>;
    };

    template<class T, class U>
    using copy_cvref_t = typename copy_cvref<T, U>::type;

    template<class T, bool = !is_tuple_like_v<T> &&
        !std::is_same_v<remove_cvref_t<T>, void*> &&
        !std::is_array_v<T>, class = void>
    struct optional_traits_impl {};

    template<class T>
    struct optional_traits_impl<T, true, std::void_t<           // not accept tuple like classes, void* and array
        decltype(static_cast<bool>(std::declval<T&>())),        // can cast to bool == std::is_convertible_v<T, bool>
        decltype(*std::declval<T&>())                           // has operator*()
    >> {
        using reference = decltype(*std::declval<T&>());
        using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
    };

    template<class T, class = void>
    struct optional_traits : optional_traits_impl<T> {};

    struct dummy_type {};
    // reflect on optional to work std::optional with predeclared classes
    template<template <class> class optional_like_t, class real_value_type>
    struct optional_traits<optional_like_t<real_value_type>, std::enable_if_t<std::is_same_v<dummy_type,
        typename optional_traits_impl<optional_like_t<dummy_type>>::value_type
    >>> {
        using reference [[maybe_unused]] = copy_cvref_t<typename optional_traits_impl<optional_like_t<dummy_type>>::reference, real_value_type>;
        using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
    };

    template<template <class> class optional_like_t, class real_value_type>
    struct optional_traits<const optional_like_t<real_value_type>, std::enable_if_t<std::is_same_v<const dummy_type,
        typename optional_traits_impl<const optional_like_t<dummy_type>>::value_type
    >>> {
        using reference [[maybe_unused]] = copy_cvref_t<typename optional_traits_impl<const optional_like_t<dummy_type>>::reference, real_value_type>;
        using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
    };

    template<class T>
    using optional_traits_type = typename optional_traits<T>::value_type;

    template<class, class = void>
    constexpr inline bool is_optional_v = false;
    template<class T>
    constexpr inline bool is_optional_v<T, std::void_t<optional_traits_type<T>>>
        = !std::is_void_v<optional_traits_type<T>>;


    template<class T, std::size_t = sizeof(T)>
    [[maybe_unused]] constexpr std::true_type defined_type(int);
    template<class>
    constexpr std::false_type defined_type(...);

    template<class T, class = void>
    struct is_defined : decltype(defined_type<T>(0)) {
    };

    template<class T>
    struct is_defined<T, std::enable_if_t<std::is_array_v<T>>> : is_defined<std::remove_extent_t<T>> {
    };

    template<class T>
    struct is_defined<T, std::enable_if_t<is_tuple_like_v<T>>> {
        template<std::size_t ...Ix>
        static constexpr bool is_all_defined(std::index_sequence<Ix...>) {
            return (is_defined<std::tuple_element_t<Ix, T>>::value && ...);
        }
        [[maybe_unused]] constexpr static bool value = is_all_defined(std::make_index_sequence<std::tuple_size_v<T>>{});
    };

    template<class T>
    struct is_defined<T, std::enable_if_t<is_optional_v<T>>> : is_defined<optional_traits_type<T>> {};

    template<class T>
    constexpr inline bool is_defined_v = is_defined<T>::value;


    template <bool IsConvertible, typename From, typename To>
    struct is_nothrow_convertible_impl : std::false_type{};
    template <typename From, typename To>
    struct is_nothrow_convertible_impl<true, From, To> {
        static void test(To) noexcept {}
        [[maybe_unused]] constexpr static inline bool value = noexcept(test(std::declval<From>()));
    };

    template <typename From, typename To>
    constexpr inline auto is_nothrow_convertible_v
        = is_nothrow_convertible_impl<std::is_convertible_v<From,To>, From, To>::value;


    template<class Helper, class T, bool is_const_v, class ...Args>
    struct member_function_invoke_result {
        template<class Res = Helper>
        Res operator()(Res (std::remove_const_t<T>::*) (Args...)) const { return *static_cast<Res*>(nullptr); }
    };
    template<class Helper, class T, class ...Args>
    struct member_function_invoke_result<Helper, T, true, Args...> {
        template<class Res = Helper>
        Res operator()(Res (std::remove_const_t<T>::*) (Args...) const) const { return *static_cast<Res*>(nullptr); }
    };

    template<class Helper, class T, class ...Args>
    constexpr inline auto member_function_invoke_result_v
        = member_function_invoke_result<Helper, std::remove_const_t<T>, std::is_const_v<T>, Args...>{};

    template<class T, class U, bool = std::is_class_v<T> && is_defined_v<T>, class = void>
    constexpr static auto has_conversion_operator = false;
    template<class T, class U>
    constexpr static auto has_conversion_operator<T, U, true, std::void_t<
        decltype(member_function_invoke_result_v<U, T>(&T::operator U))
    >> = true;

    template<class T, class U>
    constexpr static inline bool has_any_conversion_operator =
        has_conversion_operator<T, U> || has_conversion_operator<T, U&> || has_conversion_operator<T, const U&> ||
        has_conversion_operator<const T, U> || has_conversion_operator<const T, U&> || has_conversion_operator<const T, const U&>;


    template<class T, bool = is_defined_v<T>>
    constexpr inline bool is_bool_ref_v = false;
    template<class T>
    constexpr inline bool is_bool_ref_v<T, true> =  // type must be defined
        std::is_class_v<T> &&                       // bool ref can be only classes, whose
        is_nothrow_convertible_v<T, bool> &&        // can convert to bool
        has_any_conversion_operator<T, bool> &&     // with bool conversion operator
        !std::is_constructible_v<T, bool&>;         // cannot construct from bool&

    template<class T>
    constexpr inline bool is_bool_v = std::is_same_v<std::remove_cv_t<T>, bool> || is_bool_ref_v<T>;


    template<class T, bool = is_defined_v<T>>
    constexpr inline bool is_size_t_wrapper = false;
    template<class T>
    constexpr inline bool is_size_t_wrapper<T, true> =  // type must be defined
        std::is_class_v<T> &&                           // size_t wrapper can be only classes, whose
        is_nothrow_convertible_v<T, std::size_t> &&     // can convert to size_t
        !has_any_conversion_operator<T, bool> &&        // but not with operator bool. accept char-s
        is_nothrow_convertible_v<std::size_t, T>;       // can convert from size_t

    template<class T>
    constexpr inline bool is_index_v = !std::is_same_v<bool, std::remove_cv_t<T>> && !is_char_v<T> &&
        (std::is_integral_v<T> || is_size_t_wrapper<T>);


    template<class T, bool = is_defined_v<T>, class = void>
    constexpr inline bool has_size_v = false;
    template<class T>
    constexpr inline bool has_size_v<T, true, std::enable_if_t<
        is_index_v<decltype(std::size(std::declval<T&>()))>
    >> = true;


    template<class T, class With = void, bool = has_size_v<T>, class = void>
    struct subscript_operator_traits {};
    template<class T>
    struct subscript_operator_traits<T, void, true> : subscript_operator_traits<T, decltype(std::size(std::declval<T&>()))> {};
    template<class T>
    struct subscript_operator_traits<T, void, false> : subscript_operator_traits<T, std::size_t> {};
    template<class T, class With, bool has_size>
    struct subscript_operator_traits<T, With, has_size, std::void_t<
        std::enable_if_t<!std::is_void_v<With>>,
        decltype(member_function_invoke_result_v<void, T, With>(&T::operator[]))
    >> {
        using type [[maybe_unused]] = decltype(member_function_invoke_result_v<void, T, With>(&T::operator[]));
    };

    template<class T, class With = void>
    using subscript_operator_return = remove_cvref_t<typename subscript_operator_traits<std::remove_const_t<T>, With>::type>;

    template<class, class = void, class = void>
    constexpr inline bool has_subscript_operator = false;
    template<class T, class With>
    constexpr inline bool has_subscript_operator<T, With, std::void_t<
        typename subscript_operator_traits<T, With>::type
    >> = true;


    template<class T, bool = has_size_v<T> && std::is_class_v<T>, class = void>
    constexpr inline bool is_bitset_like_v = false;
    template<class T>
    constexpr inline bool is_bitset_like_v<T, true, std::enable_if_t<   // bitset has size && it is a class
        has_subscript_operator<std::remove_const_t<T>>                  // and you can index it, to get a bool_ref
    >> = is_bool_ref_v<subscript_operator_return<T>>;


    template<auto* Lambda, int=((*Lambda)(), 0)>
    [[maybe_unused]] constexpr bool is_constexpr(int) { return true; }
    template<auto*>
    constexpr bool is_constexpr(...) { return false; }

    template<class, class = void>
    constexpr inline auto constexpr_std_size = [] { throw; };
    template<class T>
    constexpr inline auto constexpr_std_size<T, std::enable_if_t<std::is_trivially_destructible_v<T>>>
        = [] { return std::size(T{}); };
    // TODO add constexpr not trivially destructible trait (C++20) if recursively declared all class
    // the main reason why it is disabled currently because if you had a predeclared class inside your container,
    // you cannot instantiate the class compile time without compile error

    template<class T, bool = has_size_v<T>, class = void>
    constexpr inline std::size_t constexpr_std_size_v = 0;
    template<class T>
    constexpr inline std::size_t constexpr_std_size_v<T, true, std::enable_if_t<
        is_constexpr<&constexpr_std_size<remove_cvref_t<T>>>(0)>
    > = constexpr_std_size<remove_cvref_t<T>>();

    template<class, class = void>
    constexpr inline std::size_t compile_time_size_v = 0;
    template<class T>
    constexpr inline std::size_t compile_time_size_v<T, std::enable_if_t<
        std::is_array_v<T>
    >> = std::extent_v<T>;
    template<class T>
    constexpr inline std::size_t compile_time_size_v<T, std::enable_if_t<
        is_tuple_like_v<T>
    >> = tuple_has_same_values_v<T> ? std::tuple_size_v<T> : 0;
    template<class T>
    constexpr inline std::size_t compile_time_size_v<T, std::enable_if_t<
        !std::is_array_v<T> && !is_tuple_like_v<T> && constexpr_std_size_v<T>
    >> = std::size(T{});


    template<class, class = void>
    constexpr inline bool has_std_iterator_traits_v = false;
    template<class It>
    constexpr inline bool has_std_iterator_traits_v<It, std::void_t<typename std::iterator_traits<It>::value_type>> = true;


    template<class It, class Sentinel, bool = has_std_iterator_traits_v<It> &&
                                              std::is_same_v<It, Sentinel>, class = void>
    constexpr inline bool is_iterator_pair_v = false;
    template<class It, class Sentinel>
    constexpr inline bool is_iterator_pair_v<It, Sentinel, true> = true;
    template<class It, class Sentinel>
    constexpr inline bool is_iterator_pair_v<It, Sentinel, false, std::enable_if_t<
        std::is_convertible_v<decltype(std::declval<It&>() != std::declval<Sentinel&>()), bool> &&
        !std::is_void_v<decltype(*std::declval<It&>())> &&
        std::is_same_v<decltype(++std::declval<It&>()), It&>
    >> = true;

    template<class T>
    using get_begin_iterator_t = remove_cvref_t<decltype(std::begin(std::declval<T&>()))>;
    template<class T>
    using get_end_iterator_t = remove_cvref_t<decltype(std::end(std::declval<T&>()))>;

    template<class T, bool = is_defined_v<T>, class = void>
    constexpr inline bool has_begin_end_iterators_v = false;
    template<class T>
    constexpr inline bool has_begin_end_iterators_v<T, true, std::enable_if_t<
        is_iterator_pair_v<get_begin_iterator_t<T>, get_end_iterator_t<T>>
    >> = true;

    template<class T, bool = has_begin_end_iterators_v<T>, class = void>
    struct range_traits_impl {};

    template<class T>
    struct range_traits_impl<T, true, std::enable_if_t<
        !has_std_iterator_traits_v<get_begin_iterator_t<T>>
    >> {
        template<class U, bool>
        struct ssize_type : std::common_type<std::ptrdiff_t> {};
        template<class U>
        struct ssize_type<U, true> : std::make_signed<remove_cvref_t<decltype(std::size(std::declval<U>()))>> {};

        using reference = decltype(*std::begin(std::declval<T&>()));
        using value_type [[maybe_unused]] = std::remove_reference_t<reference>;

        constexpr static bool is_sized = has_size_v<T> || compile_time_size_v<T>;
        [[maybe_unused]] constexpr static bool random_access =
            has_subscript_operator<const T> ||
            std::is_invocable_r_v<get_begin_iterator_t<T>, std::plus<>, get_begin_iterator_t<T>, typename ssize_type<T, is_sized>::type>;
        [[maybe_unused]] constexpr static bool predeclared_array = false;
    };

    template<class T>
    struct range_traits_impl<T, true, std::enable_if_t<
        has_std_iterator_traits_v<get_begin_iterator_t<T>>
    >> {
        using reference = typename std::iterator_traits<get_begin_iterator_t<T>>::reference;
        using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
        [[maybe_unused]] constexpr static bool is_sized = has_size_v<T> || compile_time_size_v<T>;
        [[maybe_unused]] constexpr static bool random_access =
            std::is_base_of_v<std::random_access_iterator_tag,
            typename std::iterator_traits<get_begin_iterator_t<T>>::iterator_category>;
        [[maybe_unused]] constexpr static bool predeclared_array = false;
    };

    template<class T, bool = is_tuple_like_v<T>, class = void>
    struct range_traits : range_traits_impl<T> {};

    template<class T, std::size_t Ix, bool any>
    struct range_traits<T[Ix], any> {
        using reference [[maybe_unused]] = T&;
        using value_type [[maybe_unused]] = T;
        [[maybe_unused]] constexpr static bool is_sized = true;
        [[maybe_unused]] constexpr static bool random_access = true;
        [[maybe_unused]] constexpr static bool predeclared_array = !is_defined_v<T>;
    };

    template<template <class, std::size_t> class container, class real_value_type, std::size_t Ix>
    struct range_traits<container<real_value_type, Ix>, true, std::enable_if_t<
        std::is_same_v<dummy_type, typename range_traits_impl<container<dummy_type, Ix>>::value_type>
    >> {
        using reference [[maybe_unused]] = copy_cvref_t<typename range_traits_impl<container<dummy_type, Ix>>::reference, real_value_type>;
        using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
        [[maybe_unused]] constexpr static bool is_sized = range_traits_impl<container<dummy_type, Ix>>::is_sized;
        [[maybe_unused]] constexpr static bool random_access = range_traits_impl<container<dummy_type, Ix>>::random_access;
        [[maybe_unused]] constexpr static bool predeclared_array = !is_defined_v<real_value_type>;
    };


    template<template <class, std::size_t> class container, class real_value_type, std::size_t Ix>
    struct range_traits<const container<real_value_type, Ix>, true, std::enable_if_t<
        std::is_same_v<const dummy_type, typename range_traits_impl<const container<dummy_type, Ix>>::value_type>
    >> {
        using reference [[maybe_unused]] = copy_cvref_t<typename range_traits_impl<const container<dummy_type, Ix>>::reference, real_value_type>;
        using value_type [[maybe_unused]] = std::remove_reference_t<reference>;
        [[maybe_unused]] constexpr static bool is_sized = range_traits_impl<const container<dummy_type, Ix>>::is_sized;
        [[maybe_unused]] constexpr static bool random_access = range_traits_impl<const container<dummy_type, Ix>>::random_access;
        [[maybe_unused]] constexpr static bool predeclared_array = !is_defined_v<real_value_type>;
    };

    template<class T>
    using range_traits_type = typename range_traits<T>::value_type;

    template<class, class = void>
    constexpr inline bool is_range_v = false;
    template<class T>
    constexpr inline bool is_range_v<T, std::void_t<
        typename range_traits<T>::value_type
    >> = true;


    template<class, class = void>
    constexpr inline bool is_sized_range_v = false;
    template<class T>
    constexpr inline bool is_sized_range_v<T, std::enable_if_t<is_range_v<T>>> = range_traits<T>::is_sized;


    template<class T, bool = is_range_v<T> && std::is_class_v<T> && is_defined_v<T>, class = void>
    constexpr inline bool is_string_like_v = false;
    template<class T>
    constexpr inline bool is_string_like_v<T, true, std::void_t<
        decltype(member_function_invoke_result_v<void, const T>(&T::length))
    >> = true;


    template<class T, bool = is_range_v<T>, class = void>
    constexpr inline bool is_random_access_range_v = false;
    template<class T>
    constexpr inline bool is_random_access_range_v<T, true> = range_traits<T>::random_access;


    template<class Impl, std::size_t = std::tuple_size_v<typename range_traits<Impl>::value_type>>
    struct has_map_like_properties_impl : std::false_type {};
    template<class Impl>
    struct has_map_like_properties_impl<Impl, 2> {
        template<class, class = void>
        constexpr static inline bool is_transparent_v = false;
        template<class T>
        constexpr static inline bool is_transparent_v<T, std::void_t<typename T::is_transparent>> = true;

        template<class, class = void>
        constexpr static inline bool has_no_transparent_key_comp_v = false;
        template<class T>
        constexpr static inline bool has_no_transparent_key_comp_v<T, std::void_t<
            decltype(member_function_invoke_result_v<void, const T>(&T::key_comp))
        >> = !is_transparent_v<decltype(member_function_invoke_result_v<void, const T>(&T::key_comp))>;

        template<class, class = void>
        constexpr static inline bool has_no_transparent_key_eq_v = false;
        template<class T>
        constexpr static inline bool has_no_transparent_key_eq_v<T, std::void_t<
            decltype(member_function_invoke_result_v<void, const T>(&T::key_eq))
        >> = !is_transparent_v<decltype(member_function_invoke_result_v<void, const T>(&T::key_eq))>;

        template<class, class = void>
        constexpr static inline bool has_no_transparent_hash_function_v = false;
        template<class T>
        constexpr static inline bool has_no_transparent_hash_function_v<T, std::void_t<
            decltype(member_function_invoke_result_v<void, const T>(&T::hash_function))
        >> = !is_transparent_v<decltype(member_function_invoke_result_v<void, const T>(&T::hash_function))>;


        template<class, class = void>
        constexpr static inline bool has_map_find_function_v = false;
        template<class T>
        constexpr static inline bool has_map_find_function_v<T, std::void_t<
            decltype(member_function_invoke_result_v<get_begin_iterator_t<const T>, const T, copy_cvref_t<
                typename range_traits<const T>::reference,
                std::tuple_element_t<0, typename range_traits<T>::value_type>
            >>(&T::find))
        >> = true;

        template<class, class = void>
        constexpr static inline bool has_set_with_tuple_find_function_v = false;
        template<class T>
        constexpr static inline bool has_set_with_tuple_find_function_v<T, std::void_t<
            decltype(member_function_invoke_result_v<get_begin_iterator_t<const T>, const T, typename range_traits<const T>::reference>(&T::find))
        >> = true;

        template<class, class = void>
        constexpr static inline bool has_map_at_function_v = false;
        template<class T>
        constexpr static inline bool has_map_at_function_v<T, std::enable_if_t<
            std::is_same_v<
                copy_cvref_t<
                    typename range_traits<const T>::reference,
                    std::tuple_element_t<1, typename range_traits<T>::value_type>
                >,
                decltype(member_function_invoke_result_v<copy_cvref_t<
                    typename range_traits<const T>::reference,
                    std::tuple_element_t<1, typename range_traits<T>::value_type>
                >, const T, copy_cvref_t<
                    typename range_traits<const T>::reference,
                    std::tuple_element_t<0, typename range_traits<T>::value_type>
                >>(&T::at))
            >
        >> = true;

        template<class, class = void>
        constexpr static inline bool has_map_key_type_v = false;
        template<class T>
        constexpr static inline bool has_map_key_type_v<T, std::enable_if_t<
            std::is_same_v<typename T::key_type, remove_cvref_t<std::tuple_element_t<0, typename range_traits<T>::value_type>>>
        >> = true;

        [[maybe_unused]] constexpr static inline bool value = has_map_find_function_v<Impl> &&
            (!has_set_with_tuple_find_function_v<Impl> ||
             has_map_at_function_v<Impl> ||
             has_map_key_type_v<Impl> ||
             has_no_transparent_key_comp_v<Impl> ||
             has_no_transparent_key_eq_v<Impl> ||
             has_no_transparent_hash_function_v<Impl>);
    };


    template<class T, bool =
        is_tuple_like_v<typename range_traits<T>::value_type> &&
        !range_traits<T>::predeclared_array>
    constexpr inline bool has_map_like_properties_v = false;
    template<class T>
    constexpr inline bool has_map_like_properties_v<T, true> = has_map_like_properties_impl<T>::value;

    template<class T, bool = is_sized_range_v<T> && std::is_class_v<T>>
    constexpr inline bool is_map_like_container_v = false;
    template<class T>
    constexpr inline bool is_map_like_container_v<T, true> = has_map_like_properties_v<T>;


    enum class type_classification {
        indeterminate,
        pre_declared,
        random_access_range,
        bitset_like_container,
        map_like_container,
        sized_range,
        range,
        tuple_like,
        optional,
        bool_t,
        index,
    };

    template<class T, class = void>
    constexpr inline type_classification classify = decltype(defined_type<T>(0))::value ? type_classification::indeterminate : type_classification::pre_declared;

    template<class T>
    constexpr inline type_classification classify<T,
        std::enable_if_t<is_tuple_like_v<T> && !is_range_v<T>>>
        = type_classification::tuple_like;


    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_random_access_range_v<T> &&
                                                                      !is_string_like_v<T> && !is_bitset_like_v<T>>>
        = type_classification::random_access_range;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_bitset_like_v<T>>>
        = type_classification::bitset_like_container;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_map_like_container_v<T>>>
        = type_classification::map_like_container;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_sized_range_v<T> && !is_map_like_container_v<T>
        && !is_random_access_range_v<T> && !is_string_like_v<T>>>
        = type_classification::sized_range;

    template<class T>
    constexpr inline type_classification classify<T, std::enable_if_t<is_range_v<T> && !is_sized_range_v<T> && !is_string_like_v<T>>>
        = type_classification::range;

    template<class T>
    constexpr inline type_classification classify<T,
        std::enable_if_t<is_optional_v<T>>>
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
