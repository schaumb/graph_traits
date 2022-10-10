//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_GRAPH_TRAITS_IMPL_HPP
#define BXLX_GRAPH_TRAITS_GRAPH_TRAITS_IMPL_HPP

#include <tuple>
#include <optional>
#include <vector>
#include <array>
#include <type_traits>

namespace bxlx::detail {
    enum class graph_representation {
        adjacency_list,
        adjacency_array,
        adjacency_matrix
    };

    enum class type_classification {
        indeterminate,
        random_access_range,
        range,
        tuple_like,
        optional,
        bool_t,
        integral,
        string
    };

    template<class...>
    constexpr static bool always_false = false;

    template<class T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    template<class T, class = void>
    constexpr static bool is_random_access_range = false;

    template<class T>
    constexpr static bool is_random_access_range<T, std::enable_if_t<
        std::is_integral_v<decltype(size(std::declval<T&>()))> &&
        std::is_same_v<decltype(*std::declval<T&>().begin()), decltype(std::declval<T&>()[size(std::declval<T&>())])>
    >> = true;

    template<class T>
    constexpr static bool is_char = std::is_same_v<T, char> || std::is_same_v<T, char16_t>
        || std::is_same_v<T, char32_t> || std::is_same_v<T, wchar_t> || std::is_same_v<T, decltype(u8'\0')>;

    template<class T, class = void>
    constexpr static bool is_string_like = false;

    template<class T>
    constexpr static bool is_string_like<T, std::enable_if_t<is_random_access_range<T>>> =
        is_char<remove_cvref_t<decltype(*std::declval<T&>().begin())>>;

    template<class T, class = void>
    constexpr static bool is_tuple_like = false;

    template<class T>
    constexpr static bool is_tuple_like<T, std::enable_if_t<
        sizeof(std::tuple_size<T>) != 0
    >> = true;


    template<class T, class = void>
    constexpr static type_classification classify = type_classification::indeterminate;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<is_string_like<T>>> =
        type_classification::string;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<is_random_access_range<T> &&
        !is_string_like<T>>> =
        type_classification::random_access_range;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<
        !is_random_access_range<T> &&
        std::is_integral_v<decltype(size(std::declval<T&>()))> &&
        std::is_void_v<std::void_t<decltype(*std::declval<T&>().begin())>>
    >> = type_classification::range;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<
        !is_random_access_range<T> && is_tuple_like<T>
    >> = type_classification::tuple_like;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<
        std::is_void_v<std::void_t<decltype(*std::declval<T&>()), decltype(static_cast<bool>(std::declval<T&>()))>>
    >> = type_classification::optional;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<
        std::is_same_v<T, bool> ||
        (std::is_convertible_v<T, bool> && !std::is_constructible_v<T, bool> &&
            std::is_class_v<T>)
    >> = type_classification::bool_t;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<
        std::is_integral_v<T> && !std::is_same_v<T, bool>
    >> = type_classification::integral;

    struct noop_t {
        template<class ...Ts>
        constexpr void operator()(Ts && ...) const noexcept {
        }
    };

    template<std::size_t I>
    struct getter_t {
        template<class T, class ...Ts>
        [[nodiscard]] constexpr auto operator()(T&& val, Ts&&...) const noexcept -> decltype(std::get<I>(val)) {
            return std::get<I>(val);
        }
    };

    struct indirect_t {
        template<class T, class ...Ts>
        [[nodiscard]] constexpr auto operator()(T&& val, Ts&&...) const noexcept -> decltype(*val) {
            return *val;
        }
    };

    struct identity_t {
        template<class T, class ...Ts>
        [[nodiscard]] constexpr T&& operator()(T&& val, Ts&&...) const noexcept {
            return std::forward<T>(val);
        }
    };

    template<class graph_t, type_classification like = classify<graph_t>, class = void>
    struct adjacency_array_traits {
        static_assert(always_false<graph_t>,
            "Adjacency array element must be tuple like object, where 2 or 3 component accepted.");
    };

    template<class graph_t>
    struct adjacency_array_traits<graph_t, type_classification::tuple_like, std::enable_if_t<(std::tuple_size_v<graph_t> == 2 || std::tuple_size_v<graph_t> == 3)>>
    {
        // static_assert(classify<std::tuple_element_t<0, graph_t>> == type_classification::integral, "Adjacency array 0th component must be an integer type");
        // static_assert(classify<std::tuple_element_t<1, graph_t>> == type_classification::integral, "Adjacency array 1th component must be an integer type");
        // This is needed?
        //static_assert(std::is_same_v<remove_cvref_t<std::tuple_element_t<0, graph_t>>, remove_cvref_t<std::tuple_element_t<1, graph_t>>>,
        //    "Adjacency array first 2 component type must be the same");

        constexpr static graph_representation representation = graph_representation::adjacency_array;

        constexpr static auto get_edge_property = std::conditional_t<std::tuple_size_v<graph_t> == 3, getter_t<2>, noop_t>{};
        using edge_repr_type = graph_t;

        constexpr static auto get_node_property = noop_t{};
        using node_repr_type = void*;

        using node_index_t = std::common_type_t<remove_cvref_t<std::tuple_element_t<0, graph_t>>,
            remove_cvref_t<std::tuple_element_t<1, graph_t>>>;

        constexpr static auto edge_source = getter_t<0>{};
        constexpr static auto edge_target = getter_t<1>{};

        constexpr static std::size_t storage_size = 0;
    };

    template<class graph_t, type_classification like = classify<graph_t>, class = void>
    struct adjacency_list_traits {
        static_assert(always_false<graph_t>, "Adjacency list element must be integral or 2 component tuple like object.");
    };

    template<class graph_t>
    struct adjacency_list_traits<graph_t, type_classification::tuple_like,
            std::enable_if_t<std::tuple_size_v<graph_t> == 2>>
        : adjacency_list_traits<std::tuple_element_t<0, graph_t>>
    {
        constexpr static auto get_edge_property = getter_t<1>{};
        using edge_repr_type = graph_t;

        constexpr static auto edge_target = getter_t<0>{};
    };

    template<class graph_t>
    struct adjacency_list_traits<graph_t, type_classification::integral> {
        constexpr static graph_representation representation = graph_representation::adjacency_list;

        constexpr static auto get_edge_property = noop_t{};
        using edge_repr_type = graph_t;

        constexpr static auto edge_target = identity_t{};
    };

    template<class graph_t, type_classification like = classify<graph_t>>
    struct adjacency_matrix_traits {
        static_assert(always_false<graph_t>,
            "Adjacency matrix must be contains a bool or an optional.");
    };

    template<class graph_t>
    struct adjacency_matrix_traits<graph_t, type_classification::bool_t> {
        constexpr static graph_representation representation = graph_representation::adjacency_matrix;

        constexpr static auto get_edge_property = noop_t{};
        using edge_repr_type = graph_t;
    };

    template<class graph_t>
    struct adjacency_matrix_traits<graph_t, type_classification::optional> {
        constexpr static graph_representation representation = graph_representation::adjacency_matrix;

        constexpr static auto get_edge_property = indirect_t{};
        using edge_repr_type = graph_t;
    };


    template<class graph_t, type_classification like = classify<graph_t>, class = void, bool tuple_like = is_tuple_like<graph_t>>
    struct graph_element_traits {
        static_assert(always_false<graph_t>, "graph_t elements must be range or tuple.");
    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::tuple_like,
        std::enable_if_t<(classify<std::tuple_element_t<0, graph_t>> != type_classification::range &&
            classify<std::tuple_element_t<0, graph_t>> != type_classification::random_access_range) ||
            std::tuple_size_v<graph_t> == 3>, true>
        : adjacency_array_traits<graph_t>
    {

    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::tuple_like,
        std::enable_if_t<(classify<std::tuple_element_t<0, graph_t>> == type_classification::range ||
                              classify<std::tuple_element_t<0, graph_t>> == type_classification::random_access_range) &&
                        std::tuple_size_v<graph_t> == 2>, true>
        : graph_element_traits<std::tuple_element_t<0, graph_t>>
    {
        constexpr static auto get_node_property = getter_t<1>{};
        using node_repr_type = graph_t;

        constexpr static auto out_edges = getter_t<0>{};
    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::range, void, false>
        : adjacency_list_traits<std::remove_reference_t<decltype(*std::declval<graph_t&>().begin())>>
    {
        using node_index_t = decltype(size(std::declval<graph_t&>()));
        constexpr static auto out_edges = identity_t{};
        constexpr static auto get_node_property = noop_t{};
        using node_repr_type = graph_t;

        constexpr static std::size_t storage_size = 0;
    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::random_access_range, std::enable_if_t<
        classify<remove_cvref_t<decltype(std::declval<graph_t&>()[size(std::declval<graph_t&>())])>> == type_classification::integral ||
        classify<remove_cvref_t<decltype(std::declval<graph_t&>()[size(std::declval<graph_t&>())])>> == type_classification::tuple_like
        >, false> : adjacency_list_traits<std::remove_reference_t<decltype(std::declval<graph_t&>()[size(std::declval<graph_t&>())])>>
    {
        using node_index_t = decltype(size(std::declval<graph_t&>()));
        constexpr static auto out_edges = identity_t{};
        constexpr static auto get_node_property = noop_t{};
        using node_repr_type = graph_t;

        constexpr static std::size_t storage_size = 0;
    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::random_access_range, std::enable_if_t<
        classify<remove_cvref_t<decltype(std::declval<graph_t&>()[size(std::declval<graph_t&>())])>> == type_classification::bool_t ||
        classify<remove_cvref_t<decltype(std::declval<graph_t&>()[size(std::declval<graph_t&>())])>> == type_classification::optional
    >, false> : adjacency_matrix_traits<std::remove_reference_t<decltype(std::declval<graph_t&>()[size(std::declval<graph_t&>())])>>
    {
        using node_index_t = decltype(size(std::declval<graph_t&>()));
        constexpr static auto out_edges = identity_t{};
        constexpr static auto get_node_property = noop_t{};
        using node_repr_type = graph_t;

        constexpr static std::size_t storage_size = 0;
    };

    template<class graph_t, type_classification like>
    struct graph_element_traits<graph_t, like, std::enable_if_t<like == type_classification::random_access_range || like == type_classification::range>, true>
        : graph_element_traits<graph_t, like, void, false>
    {
        constexpr static std::size_t storage_size = std::tuple_size_v<graph_t>;
    };


    template<class graph_t, type_classification like = classify<graph_t>>
    struct graph_traits_impl {
        static_assert(always_false<graph_t>, "graph_t type is not a tuple and not a range.");
    };

    template<class graph_t>
    struct graph_traits_impl<graph_t, type_classification::tuple_like>
        : graph_traits_impl<std::tuple_element_t<0, graph_t>>
    {
        constexpr static auto get_graph_property = getter_t<1>{};
        constexpr static auto get_data = getter_t<0>{};
        using graph_repr_type = graph_t;
    };

    template<class graph_t>
    struct graph_traits_impl<graph_t, type_classification::random_access_range>
        : graph_element_traits<std::remove_reference_t<decltype(std::declval<graph_t&>()[size(std::declval<graph_t&>())])>>
    {
        constexpr static auto get_graph_property = noop_t{};
        constexpr static auto get_data = identity_t{};
        using graph_repr_type = graph_t;
    };

    template<class graph_t>
    struct graph_traits_impl<graph_t, type_classification::range>
        : adjacency_array_traits<std::remove_reference_t<decltype(*std::declval<graph_t&>().begin())>>
    {
        constexpr static auto get_graph_property = noop_t{};
        constexpr static auto get_data = identity_t{};
        using graph_repr_type = graph_t;
    };

    template<class graph_t>
    struct graph_traits : graph_traits_impl<std::remove_reference_t<graph_t>> {
        using impl = graph_traits_impl<std::remove_reference_t<graph_t>>;

        using graph_property_type = std::remove_reference_t<
            std::invoke_result_t<decltype(impl::get_graph_property), graph_t>>;
        constexpr static bool has_graph_property = !std::is_void_v<graph_property_type>;

        using node_property_type = std::remove_reference_t<
            std::invoke_result_t<decltype(impl::get_node_property), typename impl::node_repr_type>>;
        constexpr static bool has_node_property = !std::is_void_v<node_property_type>;

        using edge_property_type = std::remove_reference_t<
            std::invoke_result_t<decltype(impl::get_edge_property), typename impl::edge_repr_type>>;
        constexpr static bool has_edge_property = !std::is_void_v<edge_property_type>;

        template<char operation, std::size_t component>
        constexpr static auto transform_size(std::size_t s) {
            switch (operation) {
                case '+':
                    return s + component;
                case '*':
                    return s * component;
                case '^':
                    if (s == 2) {
                        return s * s;
                    }
                    [[fallthrough]];
                default:
                    return s;
            }
        }

        template<class T, char operation = ' ', std::size_t component = (operation == '+' ? 0 : 1)>
        using storage_t = std::conditional_t<
            (impl::storage_size > 0),
            std::array<T, transform_size<operation, component>(impl::storage_size)>,
            std::vector<T>>;

        template<class T, char operation = ' ', std::size_t component = (operation == '+' ? 0 : 1)>
        constexpr static storage_t<T, operation, component> storage_init([[maybe_unused]] const graph_t& g) {
            if constexpr(impl::storage_size > 0) {
                return {};
            } else {
                return storage_t<T, operation, component>(transform_size<operation, component>(size(impl::get_data(g))));
            }
        }

        template<class T, char operation = ' ', std::size_t component = (operation == '+' ? 0 : 1)>
        using storage_unfilled_t =
            std::conditional_t<(impl::storage_size > 0),
                std::array<std::optional<T>, transform_size<operation, component>(impl::storage_size)>,
                std::vector<T>>;

        template<class T, char operation = ' ', std::size_t component = (operation == '+' ? 0 : 1)>
        constexpr static storage_unfilled_t<T, operation, component> storage_unfilled_init([[maybe_unused]] graph_t&& g) {
            if constexpr(impl::storage_size > 0) {
                return {};
            } else {
                storage_unfilled_t<T, operation, component> res;
                res.reserve(transform_size<operation, component>(size(impl::get_data(g))));
                return res;
            }
        }

        template<class T, char operation = ' ', std::size_t component = (operation == '+' ? 0 : 1), class ...Ts>
        constexpr static typename storage_unfilled_t<T, operation, component>::iterator storage_unfilled_emplace(
            [[maybe_unused]] storage_unfilled_t<T, operation, component>& storage,
            typename storage_unfilled_t<T, operation, component>::iterator it, Ts&& ... ts) {
            if constexpr (impl::storage_size > 0) {
                it->emplace(std::forward<Ts>(ts)...);
                return std::next(it);
            } else {
                return storage.emplace(it, std::forward<Ts>(ts)...);
            }
        }
    };
}
#endif //BXLX_GRAPH_TRAITS_GRAPH_TRAITS_IMPL_HPP
