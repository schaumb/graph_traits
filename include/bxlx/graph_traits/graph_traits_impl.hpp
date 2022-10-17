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
        random_access_range = 1,
        range,
        tuple_like,
        optional,
        bool_t,
        bitset,
        integral,
        string
    };

    template<class...>
    constexpr static bool always_false = false;

    template<class T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    template<class, class = void>
    constexpr static bool is_random_access_range = false;

    template<class T>
    constexpr static bool is_random_access_range<T, std::enable_if_t<
        std::is_integral_v<decltype(std::size(std::declval<T>()))> &&
        std::is_same_v<decltype(*std::begin(std::declval<T&>())), decltype(std::declval<T&>()[std::size(std::declval<T>())])>
    >> = true;

    template<class T>
    constexpr static bool is_char = std::is_same_v<T, char> || std::is_same_v<T, char16_t>
        || std::is_same_v<T, char32_t> || std::is_same_v<T, wchar_t> || std::is_same_v<T, decltype(u8'\0')>;

    template<class, class = void>
    constexpr static bool is_string_like = false;

    template<class T>
    constexpr static bool is_string_like<T, std::enable_if_t<is_random_access_range<T>>> =
        is_char<remove_cvref_t<decltype(*std::begin(std::declval<T&>()))>>;

    template<class, class = void>
    constexpr static std::size_t tuple_like_size = 0;

    template<class T>
    constexpr static std::size_t tuple_like_size<T, std::enable_if_t<
        sizeof(std::tuple_size<T>) != 0
    >> = std::tuple_size_v<T>;

    template<class T>
    constexpr static std::size_t tuple_like_size<T, std::enable_if_t<std::is_array_v<T>>> = std::extent_v<T>;


    template<class T, class = void>
    constexpr static type_classification classify = [] () -> type_classification {
        static_assert(always_false<T>, "Unable to classify this class");
        return {};
    };

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
        std::is_integral_v<decltype(std::size(std::declval<T&>()))> &&
        std::is_void_v<std::void_t<decltype(*std::begin(std::declval<T&>()))>>
    >> = type_classification::range;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<
        !is_random_access_range<T> &&
        std::is_integral_v<decltype(std::size(std::declval<T&>()))> &&
        std::is_same_v<typename T::reference, decltype(std::declval<T&>()[std::size(std::declval<T>())])>
    >> = type_classification::bitset;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<
        !is_random_access_range<T> && (tuple_like_size<T> > 0)
    >> = type_classification::tuple_like;

    template<class T>
    constexpr static type_classification classify<T, std::enable_if_t<
        !is_random_access_range<T> &&
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

    template<class type1, class type2>
    struct composition_t {
        template<class T, class ...Ts>
        [[nodiscard]] constexpr auto operator()(T&& val, Ts&&... ts) const noexcept {
            return type1{}(type2{}(std::forward<T>(val), ts...), ts...);
        }
    };

    template<class graph_t, type_classification like = classify<graph_t>, class = void>
    struct adjacency_array_traits {
        static_assert(always_false<graph_t>,
            "Adjacency array element must be tuple like object, where 2 or 3 component accepted.");
    };

    template<class graph_t>
    struct adjacency_array_traits<graph_t, type_classification::tuple_like, std::enable_if_t<(std::tuple_size_v<graph_t> == 2 || std::tuple_size_v<graph_t> == 3) &&
        !std::is_void_v<std::common_type_t<remove_cvref_t<std::tuple_element_t<0, graph_t>>, remove_cvref_t<std::tuple_element_t<1, graph_t>>>>>>
    {
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
        constexpr static auto edge_target = getter_t<0>{};
        constexpr static auto get_edge_property = getter_t<1>{};
        using edge_repr_type = graph_t;
    };

    template<class graph_t>
    struct adjacency_list_traits<graph_t, type_classification::integral> {
        constexpr static graph_representation representation = graph_representation::adjacency_list;

        constexpr static auto edge_target = identity_t{};
        constexpr static auto get_edge_property = noop_t{};
        using edge_repr_type = graph_t;
    };

    template<class graph_t, type_classification like = classify<graph_t>>
    struct adjacency_matrix_traits {
        static_assert(always_false<graph_t>, "Adjacency matrix must be contains a bool or an optional.");
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


    template<class graph_t, type_classification like = classify<graph_t>, class = void, std::size_t = tuple_like_size<graph_t>>
    struct graph_element_traits {
        static_assert(always_false<graph_t>, "graph_t elements must be range tuple or bitset.");
    };

    template<class graph_t, std::size_t tuple_size>
    struct graph_element_traits<graph_t, type_classification::tuple_like,
        std::enable_if_t<(classify<std::tuple_element_t<0, graph_t>> != type_classification::range &&
            classify<std::tuple_element_t<0, graph_t>> != type_classification::random_access_range) ||
            tuple_size == 3>, tuple_size>
        : adjacency_array_traits<graph_t>
    {
        constexpr static std::size_t inside_storage_size = 0;
    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::tuple_like,
        std::enable_if_t<classify<std::tuple_element_t<0, graph_t>> == type_classification::range ||
                              classify<std::tuple_element_t<0, graph_t>> == type_classification::random_access_range>, 2>
        : graph_element_traits<std::tuple_element_t<0, graph_t>>
    {
        constexpr static auto out_edges = composition_t<decltype(graph_element_traits<std::tuple_element_t<0, graph_t>>::out_edges), getter_t<0>>{};
        constexpr static auto get_node_property = getter_t<1>{};
        using node_repr_type = graph_t;
    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::range, void, 0>
        : adjacency_list_traits<std::remove_reference_t<decltype(*std::begin(std::declval<graph_t&>()))>>
    {
        constexpr static auto out_edges = identity_t{};
        constexpr static auto get_node_property = noop_t{};
        using node_repr_type = graph_t;

        constexpr static std::size_t inside_storage_size = 0;
    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::random_access_range, std::enable_if_t<
        classify<remove_cvref_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>> == type_classification::integral ||
        classify<remove_cvref_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>> == type_classification::tuple_like
        >, 0> : adjacency_list_traits<std::remove_reference_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>>
    {
        constexpr static auto out_edges = identity_t{};
        constexpr static auto get_node_property = noop_t{};
        using node_repr_type = graph_t;

        constexpr static std::size_t inside_storage_size = 0;
    };

    template<class graph_t>
    struct graph_element_traits<graph_t, type_classification::random_access_range, std::enable_if_t<
        classify<remove_cvref_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>> == type_classification::bool_t ||
        classify<remove_cvref_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>> == type_classification::optional
    >, 0> : adjacency_matrix_traits<std::remove_reference_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>>
    {
        constexpr static auto out_edges = identity_t{};
        constexpr static auto get_node_property = noop_t{};
        using node_repr_type = graph_t;
        constexpr static auto node_connections_class = type_classification::random_access_range;

        constexpr static std::size_t inside_storage_size = 0;
    };

    template<class graph_t, std::size_t tuple_size>
    struct graph_element_traits<graph_t, type_classification::bitset, void, tuple_size>
        : adjacency_matrix_traits<std::remove_reference_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>>
    {
        struct bitset_iterator_getter {
            struct bitset_range;

            struct bitset_iterator {
                std::size_t ix = 0;
                graph_t* g = nullptr;

                bool operator!=(const bitset_iterator& oth) const {
                    return this->g != oth.g || this->ix != oth.ix;
                }

                bitset_iterator& operator++() {
                    ++ix;
                    return *this;
                }

                auto operator*() {
                    return (*g)[ix];
                }
            };
            struct bitset_range {
                bitset_iterator begin_, end_;

                bitset_iterator begin() const {
                    return begin_;
                }
                bitset_iterator end() const {
                    return end_;
                }
            };
            constexpr bitset_range operator()(graph_t& gr) const {
                return bitset_range{{0, &gr}, {inside_storage_size, &gr}};
            }
        } constexpr static out_edges{};
        constexpr static auto get_node_property = noop_t{};
        using node_repr_type = graph_t;
        constexpr static auto node_connections_class = type_classification::bitset;

        constexpr static std::size_t inside_storage_size = graph_t{}.size();
    };

    template<class graph_t, type_classification like, std::size_t tuple_size>
    struct graph_element_traits<graph_t, like, std::enable_if_t<(like == type_classification::random_access_range || like == type_classification::range) && (tuple_size > 0)>, tuple_size>
        : graph_element_traits<graph_t, like, void, 0>
    {
        constexpr static std::size_t inside_storage_size = tuple_size;
    };


    template<class graph_t, type_classification like = classify<graph_t>, class = void, std::size_t tuple_size = tuple_like_size<graph_t>>
    struct graph_traits_impl {
        static_assert(always_false<graph_t>, "graph_t type is not a tuple and not a range.");
    };

    template<class graph_t, std::size_t tuple_size>
    struct graph_traits_impl<graph_t, type_classification::tuple_like, void, tuple_size>
        : graph_traits_impl<std::tuple_element_t<0, graph_t>>
    {
        constexpr static auto get_data = getter_t<0>{};
        constexpr static auto get_graph_property = getter_t<1>{};
        using graph_repr_type = graph_t;
    };

    template<class graph_t>
    struct graph_traits_impl<graph_t, type_classification::random_access_range, void, 0>
        : graph_element_traits<std::remove_reference_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>>
    {
        using impl = graph_element_traits<std::remove_reference_t<decltype(std::declval<graph_t&>()[std::size(std::declval<graph_t&>())])>>;

        constexpr static auto get_data = identity_t{};
        constexpr static auto get_graph_property = noop_t{};
        using graph_repr_type = graph_t;

        struct dummy_t { using node_index_t = decltype(std::size(std::declval<graph_t&>())); };

        using node_index_t = typename std::conditional_t<
            impl::representation == graph_representation::adjacency_array, impl, dummy_t>::node_index_t;

        constexpr static std::size_t storage_size =
            impl::representation == graph_representation::adjacency_array ?
            impl::inside_storage_size : 0;
    };

    template<class graph_t>
    struct graph_traits_impl<graph_t, type_classification::range, void, 0>
        : adjacency_array_traits<std::remove_reference_t<decltype(*std::begin(std::declval<graph_t&>()))>>
    {
        constexpr static auto get_data = identity_t{};
        constexpr static auto get_graph_property = noop_t{};
        using graph_repr_type = graph_t;

        constexpr static std::size_t storage_size = 0;
    };
    template<class graph_t, type_classification like, std::size_t tuple_size>
    struct graph_traits_impl<graph_t, like, std::enable_if_t<(like == type_classification::random_access_range || like == type_classification::range) && (tuple_size > 0)>, tuple_size>
        : graph_traits_impl<graph_t, like, void, 0>
    {
        using impl = graph_traits_impl<graph_t, like, void, false>;
        constexpr static std::size_t storage_size = tuple_size;

        static_assert(impl::representation != graph_representation::adjacency_matrix || impl::inside_storage_size == 0 || impl::inside_storage_size == storage_size,
                          "Adjacency matrix storage sizes must be the same length");
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


        template<class T, class Op = std::plus<>, std::size_t count = 0>
        using storage_t = std::conditional_t<
            (impl::storage_size > 0),
            std::conditional_t<std::is_same_v<T, bool>,
                std::bitset<Op{}(impl::storage_size, count)>,
                std::array<T, Op{}(impl::storage_size, count)>>,
            std::vector<T>>;

        template<class T, class Op = std::plus<>, std::size_t count = 0>
        constexpr static storage_t<T, Op, count> storage_init([[maybe_unused]] const graph_t& g) {
            if constexpr(impl::storage_size > 0) {
                return {};
            } else {
                return storage_t<T, Op, count>(Op{}(std::size(impl::get_data(g)), count));
            }
        }

        template<class T, class Op = std::plus<>, std::size_t count = 0>
        using storage_unfilled_t =
            std::conditional_t<(impl::storage_size > 0),
                std::array<std::optional<T>, Op{}(impl::storage_size, count)>,
                std::vector<T>>;

        template<class T, class Op = std::plus<>, std::size_t count = 0>
        constexpr static storage_unfilled_t<T, Op, count> storage_unfilled_init([[maybe_unused]] graph_t&& g) {
            if constexpr(impl::storage_size > 0) {
                return {};
            } else {
                storage_unfilled_t<T, Op, count> res;
                res.reserve(Op{}(std::size(impl::get_data(g)), count));
                return res;
            }
        }

        template<class T, class Op = std::plus<>, std::size_t count = 0, class ...Ts>
        constexpr static typename storage_unfilled_t<T, Op, count>::iterator storage_unfilled_emplace(
            [[maybe_unused]] storage_unfilled_t<T, Op, count>& storage,
            typename storage_unfilled_t<T, Op, count>::iterator it, Ts&& ... ts) {
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
