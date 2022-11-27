//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef GRAPH_GRAPH_TRAITS_HPP
#define GRAPH_GRAPH_TRAITS_HPP

#include "type_classification.hpp"
#include <algorithm>

namespace bxlx::traits {
    template<class T, class V = std::make_index_sequence<std::tuple_size_v<T>>>
    struct properties;
    template<class T, std::size_t ...ix>
    struct properties<T, std::index_sequence<ix...>> {
        using type [[maybe_unused]] = T;
        template<class U, class V>
        constexpr static inline bool has_property_any = std::is_same_v<U, std::tuple_element_t<0, V>>;
        template<class U>
        constexpr static inline bool has_property = (has_property_any<U, std::tuple_element_t<ix, T>> || ...);

        template<class U, class V>
        constexpr static inline bool is_not_contradict_props =
            !std::is_same_v<std::tuple_element_t<0, U>, std::tuple_element_t<0, V>> ||
            std::is_same_v<std::tuple_element_t<1, U>, std::tuple_element_t<1, V>>;
        template<class U>
        constexpr static inline bool is_valid_prop = (is_not_contradict_props<U, std::tuple_element_t<ix, T>> && ...);
        constexpr static inline bool is_valid = (is_valid_prop<std::tuple_element_t<ix, T>> && ...);

        template<class U>
        constexpr static inline std::size_t get_property_index =
            std::min({(has_property_any<U, std::tuple_element_t<ix, T>> ? ix : ~std::size_t{})...});

        template<class ... Others>
        using merged_type [[maybe_unused]] = properties<decltype(std::tuple_cat(std::declval<T>(), std::declval<typename Others::type>()...))>;
    };

    using empty_properties = properties<std::tuple<>>;
    template<class K, class U>
    using property = properties<std::tuple<std::pair<K, U>>>;
    template<class Prop, class...Others>
    using merge_properties = typename Prop::template merged_type<Others...>;
    template<class Type, class U>
    using get_properties = typename Type::template properties<U>;
    template<class Type, class Has>
    constexpr static bool has_property = Type::template has_property<Has>;
    template<auto p>
    using constant_t = std::integral_constant<decltype(p), p>;
    template<class Type, class U>
    using get_property = std::tuple_element_t<1, std::tuple_element_t<Type::template get_property_index<U>, typename Type::type>>;

    template<class P, class Prop, class Or, class = void>
    struct has_property_or : std::common_type<Or> {};
    template<class P, class Prop, class Or>
    struct has_property_or<P, Prop, Or, std::enable_if_t<
        has_property<P, Prop>
    >> : std::common_type<get_property<P, Prop>> {};
    template<class P, class Prop, class Or>
    using has_property_or_t = typename has_property_or<P, Prop, Or>::type;


    enum class graph_representation {
        adjacency_list,
        adjacency_matrix,
        edge_list,
    };

    template<detail2::type_classification ... types>
    struct accept_only {
        template<class T>
        constexpr static bool is_valid() {
            return ((detail2::classify<T> == types) || ...);
        }
    };

    template<class nested, detail2::type_classification ... types>
    struct accept_recursively {
        template<class T>
        constexpr static bool is_valid() {
            if constexpr (accept_only<types...>::template is_valid<T>()) {
                return nested::template is_valid_nested<T>();
            } else {
                return false;
            }
        }
    };

    struct always_valid {
        template<class>
        constexpr static bool is_valid() {
            return true;
        }
    };

    // properties
    struct user_node_index;
    struct node_repr_type;
    struct edge_repr_type;
    struct index_container_size;
    struct inside_container_size;
    struct has_dynamic_container_size;

    struct no_prop {
        template<class>
        using properties [[maybe_unused]] = empty_properties;
    };

    template<class ReprType, class Nested>
    struct save_type : Nested {
        template<class U>
        using properties [[maybe_unused]] = merge_properties<get_properties<Nested, U>,
            property<ReprType, U>>;
    };

    struct edge_property : always_valid {
        template<class T>
        using properties [[maybe_unused]] = property<edge_property, T>;
    };
    struct node_property : always_valid {
        template<class T>
        using properties [[maybe_unused]] = property<node_property, T>;
    };
    struct graph_property : always_valid {
        template<class T>
        using properties [[maybe_unused]] = property<graph_property, T>;
    };

    struct node_index : always_valid {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<property<user_node_index, std::true_type>, property<node_index, T>>;
    };

    struct index : accept_only<detail2::type_classification::index> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<property<user_node_index, std::false_type>, property<node_index, T>>;
    };

    struct bool_t : accept_only<detail2::type_classification::bool_t>, no_prop {};
    struct bitset : accept_only<detail2::type_classification::bitset_like_container> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<
            std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0),
                property<index_container_size, constant_t<bxlx::detail2::compile_time_size_v<T>>>,
                property<has_dynamic_container_size, constant_t<0>>
            >,
            property<edge_repr_type, detail2::subscript_operator_return<T>>
        >;
    };

    template<class Cond>
    struct optional : accept_recursively<optional<Cond>, detail2::type_classification::optional> {
        template<class T>
        constexpr static bool is_valid_nested() {
            return Cond::template is_valid<detail2::optional_traits_type<T>>();
        }

        template<class T>
        using properties [[maybe_unused]] = get_properties<Cond, detail2::optional_traits_type<T>>;
    };


    template<class... Conditions>
    struct any_of {
        template<class T>
        constexpr static bool is_valid() {
            return (Conditions::template is_valid<T>() + ...) == 1;
        }

        template<class T>
        constexpr static std::size_t the_valid_index() {
            std::size_t ix{};
            std::size_t res = ~std::size_t{};
            static_cast<void>((..., (Conditions::template is_valid<T>() ? res = ix : ++ix)));
            return res;
        }

        template<class T>
        using properties [[maybe_unused]] = get_properties<std::tuple_element_t<the_valid_index<T>(), std::tuple<Conditions...>>, T>;

        template<class T>
        using graph_traits = typename std::tuple_element_t<the_valid_index<T>(), std::tuple<Conditions...>>::template graph_traits<T>;
    };

    template<class ...Conditions>
    struct tuple_like : accept_recursively<tuple_like<Conditions...>, detail2::type_classification::tuple_like> {
        template<class T, std::size_t...Ix>
        static merge_properties<empty_properties, get_properties<Conditions, std::tuple_element_t<Ix, T>>...>
        merge_properties_helper(std::index_sequence<Ix...>) { return {}; }

        template<class T>
        using properties = decltype(merge_properties_helper<T>(std::make_index_sequence<std::tuple_size_v<T>>{}));

        template<class T, std::size_t ...Ix>
        constexpr static bool is_valid_nested(std::index_sequence<Ix...>) {
            return (Conditions::template is_valid<std::tuple_element_t<Ix, T>>() && ...);
        }

        template<class T>
        constexpr static bool is_valid_nested() {
            if constexpr (sizeof...(Conditions) == std::tuple_size_v<T>) {
                if constexpr (is_valid_nested<T>(std::make_index_sequence<std::tuple_size_v<T>>{})) {
                    return properties<T>::is_valid;
                }
            }
            return false;
        }
    };

    template<class container_size_prop, class Cond>
    struct range_impl {
        template<class T>
        constexpr static bool is_valid_nested() {
            if constexpr (Cond::template is_valid<detail2::range_traits_type<T>>()) {
                return properties<T>::is_valid;
            }
            return false;
        }

        template<class T>
        using properties = merge_properties<get_properties<Cond, detail2::range_traits_type<T>>,
            std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0),
                property<container_size_prop, constant_t<bxlx::detail2::compile_time_size_v<T>>>,
                property<has_dynamic_container_size, constant_t<0>>
            >
        >;
    };

    template<class T>
    struct range : accept_recursively<range_impl<inside_container_size, T>,
        detail2::type_classification::range,
        detail2::type_classification::sized_range,
        detail2::type_classification::random_access_range>, range_impl<inside_container_size, T> {};

    template<class T>
    struct sized_range : accept_recursively<range_impl<index_container_size, T>,
        detail2::type_classification::sized_range,
        detail2::type_classification::random_access_range>, range_impl<index_container_size, T> {};

    template<class T>
    struct indexed_range : accept_recursively<indexed_range<T>,
        detail2::type_classification::random_access_range> {

        template<class U>
        using properties = merge_properties<get_properties<range_impl<index_container_size, T>, U>, property<user_node_index, std::false_type>>;

        template<class U>
        constexpr static bool is_valid_nested() {
            if constexpr (range_impl<index_container_size, T>::template is_valid_nested<U>()) {
                return properties<U>::is_valid;
            }
            return false;
        }
    };

    template<class Property, class T, class ...Ts>
    using with_property = any_of<tuple_like<T, Ts..., Property>,
        std::conditional_t<sizeof...(Ts) == 0, T, tuple_like<T, Ts...>>>;

    template<class T>
    using with_graph_property = with_property<graph_property, T>;
    template<class T>
    struct with_node_property : with_property<node_property, T> {
        template<class U>
        using properties [[maybe_unused]] = merge_properties<get_properties<with_property<node_property, T>, U>,
            property<node_repr_type, U>>;
    };
    template<class T, class ...Ts>
    struct with_edge_property : with_property<edge_property, T, Ts...> {
        template<class U>
        using properties [[maybe_unused]] = merge_properties<get_properties<with_property<edge_property, T, Ts...>, U>,
            property<edge_repr_type, U>>;
    };

    template<class Property, class SizeProperty, class T, class U>
    struct map_save : accept_recursively<range_impl<SizeProperty, save_type<Property, tuple_like<T, U>>>,
        detail2::type_classification::map_like_container>, range_impl<SizeProperty, save_type<Property, tuple_like<T, U>>> {};

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
        [[nodiscard]] constexpr auto operator()(T&& val, Ts&&... ts) const noexcept -> std::invoke_result_t<type1, std::invoke_result_t<type2, T&&, Ts&&...>, Ts&&...> {
            return type1{}(type2{}(std::forward<T>(val), ts...), ts...);
        }
    };

    template<class Props>
    struct graph_traits_common {
        constexpr static bool has_graph_property = has_property<Props, graph_property>;
        constexpr static bool has_edge_property = has_property<Props, edge_property>;
        constexpr static bool has_node_property = has_property<Props, node_property>;

        constexpr static auto get_graph_property = std::conditional_t<has_graph_property, getter_t<1>, noop_t>{};
        constexpr static auto get_data = std::conditional_t<has_graph_property, getter_t<0>, identity_t>{};

        using node_index_t [[maybe_unused]] = has_property_or_t<Props, node_index, std::size_t>;

        using edge_repr_type = get_property<Props, edge_repr_type>;

        constexpr static auto container_size = has_property_or_t<Props, index_container_size, constant_t<0>>::value;
        constexpr static auto in_container_size = has_property_or_t<Props, inside_container_size, constant_t<0>>::value;
    };

    struct adjacency_list : with_graph_property<any_of<
        indexed_range<with_node_property<range<with_edge_property<index>>>>,
        map_save<node_repr_type, index_container_size, node_index, with_property<node_property, any_of<
            range<with_edge_property<node_index>>,
            map_save<edge_repr_type, inside_container_size, node_index, edge_property>
        >>>
    >> {
        template<class T, class Props = get_properties<adjacency_list, T>>
        struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {
            constexpr static inline auto representation = graph_representation::adjacency_list;
            using node_repr_type = get_property<Props, node_repr_type>;

            constexpr static inline auto max_node_compile_time = graph_traits::container_size;
            constexpr static inline auto max_edge_compile_time = max_node_compile_time
                * (graph_traits::in_container_size == 0 ? max_node_compile_time :
                   std::min<decltype(max_node_compile_time)>(graph_traits::in_container_size, max_node_compile_time));

            template<class P, bool C, class = void>
            struct get_node_property_getter : std::common_type<
                std::conditional_t<C, composition_t<getter_t<1>, getter_t<1>>, noop_t>> {};

            template<class P>
            struct get_node_property_getter<P, true, std::enable_if_t<
                std::is_same_v<get_property<P, node_property>,
                    std::tuple_element_t<1, get_property<P, traits::node_repr_type>>
            >>> : std::common_type<getter_t<1>> {};

            constexpr static auto get_edge_property = std::conditional_t<graph_traits::has_edge_property, getter_t<1>, noop_t>{};
            constexpr static auto get_node_property = typename get_node_property_getter<Props, graph_traits::has_node_property>::type{};

            constexpr static inline auto edge_target = std::conditional_t<graph_traits::has_edge_property, getter_t<0>, identity_t>{};
            constexpr static inline auto out_edges = std::conditional_t<graph_traits::has_node_property, getter_t<0>, identity_t>{};
        };
    };

    struct adjacency_matrix : with_graph_property<
        indexed_range<with_node_property<any_of<
            indexed_range<save_type<edge_repr_type, any_of<
                bool_t,
                optional<edge_property>
            >>>,
            bitset
        >>>
    > {
        template<class T, class Props = get_properties<adjacency_matrix, T>>
        struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {
            constexpr static inline auto representation = graph_representation::adjacency_matrix;
            using node_repr_type = get_property<Props, node_repr_type>;
            constexpr static inline auto max_node_compile_time = graph_traits::container_size;
            constexpr static inline auto max_edge_compile_time = max_node_compile_time * max_node_compile_time;

            constexpr static auto get_edge_property = std::conditional_t<graph_traits::has_edge_property, indirect_t, noop_t>{};
            constexpr static auto get_node_property = std::conditional_t<graph_traits::has_node_property, getter_t<1>, noop_t>{};

            constexpr static inline auto out_edges = std::conditional_t<graph_traits::has_node_property, getter_t<0>, identity_t>{};
        };
    };

    struct edge_list : with_graph_property<sized_range<
        with_edge_property<node_index, node_index>
    >> {
        template<class T, class Props = get_properties<edge_list, T>>
        struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {
            constexpr static inline auto representation = graph_representation::edge_list;
            using node_repr_type [[maybe_unused]] = void*;
            constexpr static inline auto max_edge_compile_time = graph_traits::container_size;
            constexpr static inline auto max_node_compile_time = max_edge_compile_time * 2;

            constexpr static auto get_edge_property = std::conditional_t<graph_traits::has_edge_property, getter_t<2>, noop_t>{};
            constexpr static auto get_node_property = noop_t{};

            constexpr static inline auto edge_source = getter_t<0>{};
            constexpr static inline auto edge_target = getter_t<1>{};
        };
    };

    struct graph : any_of<adjacency_list, adjacency_matrix, edge_list> {};

    template<class T>
    constexpr inline bool is_graph_v = graph::template is_valid<T>();

    template<class T>
    using graph_traits = typename graph::template graph_traits<T>;
}

#endif //GRAPH_GRAPH_TRAITS_HPP
