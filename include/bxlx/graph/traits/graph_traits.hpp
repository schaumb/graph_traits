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

namespace bxlx {
    // simplified errors
    struct not_matching_types {};
    template<class, class, class>
    struct multiple_property_for {};
    struct graph_multiple_recognize {};

    template<class Why>
    constexpr static inline bool why_not_graph = false;
}

namespace bxlx::traits {
    template<class T>
    struct type_identity { using type = T; };
    template<std::size_t I, class T>
    struct tuple_element_cvref {
        using type = detail2::copy_cvref_t<T, std::tuple_element_t<I, detail2::remove_cvref_t<T>>>;
    };
    template<std::size_t I, class T>
    using tuple_element_cvref_t = typename tuple_element_cvref<I, T>::type;

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

        constexpr static inline std::size_t get_first_invalid_prop_index =
            std::min({(!is_valid_prop<std::tuple_element_t<ix, T>> ? ix : ~std::size_t{})...});

        template<class U>
        constexpr static inline std::size_t get_last_property_index =
            std::max({(has_property_any<U, std::tuple_element_t<ix, T>> ? ix : std::size_t{})...});
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
    template<class Type,
        class U = std::tuple_element_t<0, std::tuple_element_t<Type::get_first_invalid_prop_index, typename Type::type>>>
    using get_property_error = bxlx::multiple_property_for<U, get_property<Type, U>,
        std::tuple_element_t<1, std::tuple_element_t<Type::template get_last_property_index<U>, typename Type::type>>>;

    template<class P, class Prop, class Or, class = void>
    struct has_property_or : type_identity<Or> {};
    template<class P, class Prop, class Or>
    struct has_property_or<P, Prop, Or, std::enable_if_t<
        has_property<P, Prop>
    >> : type_identity<get_property<P, Prop>> {};
    template<class P, class Prop, class Or>
    using has_property_or_t = typename has_property_or<P, Prop, Or>::type;


    namespace error_handling {
        template<class T, class...>
        struct reduce_errors {
            using type = T;
        };

        template<class T, class U, class... Oth>
        struct reduce_errors<T, U, Oth...> :
            reduce_errors<std::conditional_t<std::is_void_v<T> || std::is_same_v<T, not_matching_types> ||
                                             std::is_same_v<U, graph_multiple_recognize>, U, T>, Oth...> {};
        template<class ...Ts>
        using reduce_errors_t = typename reduce_errors<void, Ts...>::type;

        // detailed errors
        template<class>
        struct type_classification_mismatch {
            using simplified = not_matching_types;
        };
        template<class, class Properties>
        struct property_mismatch {
            using simplified = get_property_error<Properties>;
        };
        template<class, class...>
        struct multiple_graph_representation {
            using simplified = graph_multiple_recognize;
        };
        template<class>
        struct tuple_size_mismatch {
            using simplified = not_matching_types;
        };

        template<class ... All>
        struct any_nested_mismatched {
            using simplified = reduce_errors_t<typename All::simplified...>;
        };
        template<class ... All>
        struct tuple_nested_mismatched {
            using simplified = reduce_errors_t<typename All::simplified...>;
        };
        template<class, class Err>
        struct condition_and_error {
            using simplified = typename Err::simplified;
        };
        template<class C>
        struct condition_and_error<C, void> {
            using simplified = void;
        };
    }

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

        template<class T>
        constexpr static auto why_not() {
            if (!is_valid<T>()) {
                return error_handling::type_classification_mismatch<T>{};
            }
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

        template<class T, class U = nested>
        constexpr static auto why_not() {
            if constexpr (!is_valid<T>()) {
                if constexpr (accept_only<types...>::template is_valid<T>()) {
                    return U::template why_not_nested<T>();
                } else {
                    return accept_only<types...>::template why_not<T>();
                }
            }
        }
    };

    struct always_valid {
        template<class>
        constexpr static bool is_valid() {
            return true;
        }

        template<class>
        constexpr static void why_not() {}
    };

    // properties
    struct user_node_index;
    struct node_repr_type;
    struct edge_repr_type;
    struct node_container_size;
    struct edge_container_size;
    struct inside_container_size;

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

    struct node_index : accept_only<detail2::type_classification::indeterminate,
            detail2::type_classification::index, detail2::type_classification::optional> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<property<user_node_index, std::true_type>, property<node_index, std::remove_cv_t<T>>>;
    };

    struct index : accept_only<detail2::type_classification::index> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<property<user_node_index, std::false_type>, property<node_index, std::remove_cv_t<T>>>;
    };

    struct bool_t : accept_only<detail2::type_classification::bool_t>, no_prop {};
    struct bitset : accept_only<detail2::type_classification::bitset_like_container> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<
            std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0),
                property<node_container_size, constant_t<bxlx::detail2::compile_time_size_v<T>>>,
                empty_properties
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

        template<class T>
        constexpr static auto why_not_nested() {
            return Cond::template why_not<detail2::optional_traits_type<T>>();
        }
    };


    template<class... Conditions>
    struct any_of {
        template<class T>
        constexpr static bool is_valid() {
            return (Conditions::template is_valid<T>() + ...) == 1;
        }


        template<class T>
        constexpr static auto why_not() {
            if constexpr (!is_valid<T>()) {
                if constexpr (((Conditions::template is_valid<T>() + ...) > 1)) {
                    return error_handling::multiple_graph_representation<T,
                        error_handling::condition_and_error<Conditions, constant_t<Conditions::template is_valid<T>()>>...>{};
                } else {
                    return error_handling::any_nested_mismatched<error_handling::condition_and_error<Conditions, decltype(Conditions::template why_not<T>())>...>{};
                }
            }
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
        static merge_properties<empty_properties, get_properties<Conditions, tuple_element_cvref_t<Ix, T>>...>
        merge_properties_helper(std::index_sequence<Ix...>) { return {}; }

        template<class T>
        using properties = decltype(merge_properties_helper<T>(std::make_index_sequence<std::tuple_size_v<T>>{}));

        template<class T, std::size_t ...Ix>
        constexpr static bool is_valid_nested(std::index_sequence<Ix...>) {
            return (Conditions::template is_valid<tuple_element_cvref_t<Ix, T>>() && ...);
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


        template<class T, std::size_t ...Ix>
        constexpr static auto why_not_nested(std::index_sequence<Ix...>) {
            return error_handling::tuple_nested_mismatched<error_handling::condition_and_error<Conditions, decltype(Conditions::template why_not<tuple_element_cvref_t<Ix, T>>())>...>{};
        }

        template<class T>
        constexpr static auto why_not_nested() {
            if constexpr (sizeof...(Conditions) == std::tuple_size_v<T>) {
                if constexpr (is_valid_nested<T>(std::make_index_sequence<std::tuple_size_v<T>>{})) {
                    return error_handling::property_mismatch<T, get_properties<tuple_like, T>>{};
                } else {
                    return why_not_nested<T>(std::make_index_sequence<std::tuple_size_v<T>>{});
                }
            } else {
                return error_handling::tuple_size_mismatch<T>{};
            }
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
        constexpr static auto why_not_nested() {
            if constexpr (Cond::template is_valid<detail2::range_traits_type<T>>()) {
                return error_handling::property_mismatch<T, get_properties<range_impl, T>>{};
            } else {
                return Cond::template why_not<detail2::range_traits_type<T>>();
            }
        }

        template<class T>
        using properties = merge_properties<get_properties<Cond, detail2::range_traits_type<T>>,
            std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0),
                property<container_size_prop, constant_t<bxlx::detail2::compile_time_size_v<T>>>,
                empty_properties
            >
        >;
    };

    template<class T>
    struct range : accept_recursively<range_impl<inside_container_size, T>,
        detail2::type_classification::range,
        detail2::type_classification::sized_range,
        detail2::type_classification::random_access_range>, range_impl<inside_container_size, T> {};

    template<class T>
    struct edge_range : accept_recursively<range_impl<edge_container_size, T>,
        detail2::type_classification::sized_range,
        detail2::type_classification::random_access_range>, range_impl<edge_container_size, T> {};

    template<class T>
    struct node_indexed_range : accept_recursively<node_indexed_range<T>,
        detail2::type_classification::random_access_range> {

        template<class U>
        using properties = merge_properties<get_properties<range_impl<node_container_size, T>, U>, property<user_node_index, std::false_type>>;

        template<class U>
        constexpr static bool is_valid_nested() {
            if constexpr (range_impl<node_container_size, T>::template is_valid_nested<U>()) {
                return properties<U>::is_valid;
            }
            return false;
        }

        template<class U>
        constexpr static auto why_not_nested() {
            if constexpr (range_impl<node_container_size, T>::template is_valid_nested<U>()) {
                return error_handling::property_mismatch<T, get_properties<node_indexed_range, T>>{};
            } else {
                return range_impl<node_container_size, T>::template why_not_nested<U>();
            }
        }
    };

    template<class Property, class T, class ...Ts>
    using with_property = any_of<tuple_like<T, Ts..., Property>,
        std::conditional_t<sizeof...(Ts) == 0, T, tuple_like<T, Ts...>>>;

    template<class T, class ...Ts>
    using with_graph_property = with_property<graph_property, T, Ts...>;
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

    template<class U>
    using node_map_save = map_save<node_repr_type, node_container_size, node_index, U>;

    struct noop_t {
        template<class ...Ts>
        constexpr void operator()(Ts && ...) const noexcept {
        }
    };

    template<std::size_t I>
    struct getter_t {
        template<class T, class ...Ts>
        [[nodiscard]] constexpr auto operator()(T&& val, Ts&&...) const noexcept -> tuple_element_cvref_t<I, T> {
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

        using node_index_t [[maybe_unused]] = has_property_or_t<Props, node_index, std::size_t>;
        constexpr static bool user_node_index = get_property<Props, traits::user_node_index>::value;

        using node_repr_type [[maybe_unused]] = has_property_or_t<Props, traits::node_repr_type, void>;
        using edge_repr_type [[maybe_unused]] = get_property<Props, edge_repr_type>;

        constexpr static auto edge_container_size = has_property_or_t<Props, traits::edge_container_size, constant_t<0>>::value;

        using graph_property_type [[maybe_unused]] = has_property_or_t<Props, graph_property, void>;
        using node_property_type [[maybe_unused]] = has_property_or_t<Props, node_property, void>;
        using edge_property_type [[maybe_unused]] = has_property_or_t<Props, edge_property, void>;
    };

    struct adjacency_list : with_graph_property<any_of<
        node_indexed_range<with_node_property<range<with_edge_property<index>>>>,
        node_map_save<with_property<node_property, any_of<
            range<with_edge_property<node_index>>,
            map_save<edge_repr_type, inside_container_size, node_index, edge_property>
        >>>
    >> {
        template<class T, class Props = get_properties<adjacency_list, T>>
        struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {
            constexpr static inline auto representation = graph_representation::adjacency_list;

            constexpr static auto node_container_size = has_property_or_t<Props, traits::node_container_size, constant_t<0>>::value;
            constexpr static auto out_edge_container_size = has_property_or_t<Props, inside_container_size, constant_t<0>>::value;

            constexpr static inline auto max_node_compile_time = node_container_size;
            constexpr static inline auto max_edge_compile_time = max_node_compile_time
                * out_edge_container_size;

            template<class P, bool C, class = void>
            struct get_node_property_getter : type_identity<
                std::conditional_t<C, composition_t<getter_t<1>, getter_t<1>>, noop_t>> {};

            template<class P>
            struct get_node_property_getter<P, true, std::enable_if_t<
                std::is_same_v<get_property<P, node_property>,
                    tuple_element_cvref_t<1, get_property<P, traits::node_repr_type>>
            >>> : type_identity<getter_t<1>> {};

            constexpr static auto get_edge_property = std::conditional_t<graph_traits::has_edge_property, getter_t<1>, noop_t>{};
            constexpr static auto get_node_property = typename get_node_property_getter<Props, graph_traits::has_node_property>::type{};

            constexpr static inline auto edge_target = std::conditional_t<graph_traits::has_edge_property, getter_t<0>, identity_t>{};
            constexpr static inline auto out_edges = std::conditional_t<graph_traits::has_node_property, getter_t<0>, identity_t>{};

            constexpr static inline auto get_edges = noop_t{};
            constexpr static inline auto get_nodes = std::conditional_t<graph_traits::has_graph_property, getter_t<0>, identity_t>{};

            template<class, class = void>
            constexpr static inline std::nullptr_t invalid_v = nullptr;
            template<class Traits>
            constexpr static inline typename Traits::node_index_t invalid_v<Traits, std::enable_if_t<
                !Traits::user_node_index
            >> = static_cast<typename Traits::node_index_t>(~std::size_t{});

            constexpr static inline auto invalid = invalid_v<graph_traits>;

            using node_container_type = std::remove_reference_t<decltype(get_nodes(std::declval<T>()))>;
            using edge_container_type = void;
            using out_edge_container_type = std::remove_reference_t<decltype(out_edges(std::declval<typename graph_traits::node_repr_type>()))>;
        };
    };

    struct adjacency_matrix : with_graph_property<
        node_indexed_range<with_node_property<any_of<
            node_indexed_range<save_type<edge_repr_type, any_of<
                bool_t,
                optional<edge_property>
            >>>,
            bitset
        >>>
    > {
        template<class T, class Props = get_properties<adjacency_matrix, T>>
        struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {
            constexpr static inline auto representation = graph_representation::adjacency_matrix;

            constexpr static inline auto out_edges = std::conditional_t<graph_traits::has_node_property, getter_t<0>, identity_t>{};

            constexpr static inline auto get_edges = noop_t{};
            constexpr static inline auto get_nodes = std::conditional_t<graph_traits::has_graph_property, getter_t<0>, identity_t>{};

            using node_container_type = std::remove_reference_t<decltype(get_nodes(std::declval<T>()))>;
            using edge_container_type = void;
            using out_edge_container_type = std::remove_reference_t<decltype(out_edges(std::declval<typename graph_traits::node_repr_type>()))>;

            constexpr static auto node_container_size = detail2::compile_time_size_v<node_container_type>;
            constexpr static auto out_edge_container_size = detail2::compile_time_size_v<out_edge_container_type>;

            constexpr static inline auto max_node_compile_time = std::max(node_container_size, out_edge_container_size);
            constexpr static inline auto max_edge_compile_time = max_node_compile_time * max_node_compile_time;

            constexpr static auto get_edge_property = std::conditional_t<graph_traits::has_edge_property, indirect_t, noop_t>{};
            constexpr static auto get_node_property = std::conditional_t<graph_traits::has_node_property, getter_t<1>, noop_t>{};

        };
    };

    struct edge_list : any_of<
        with_graph_property<edge_range<with_edge_property<node_index, node_index>>>,
        with_graph_property<node_map_save<node_property>, edge_range<with_edge_property<node_index, node_index>>>,
        with_graph_property<node_indexed_range<save_type<node_repr_type, node_property>>, edge_range<with_edge_property<index, index>>>
    > {
        template<class T, class Props = get_properties<edge_list, T>>
        struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {
            constexpr static inline auto representation = graph_representation::edge_list;

            constexpr static auto node_container_size = has_property_or_t<Props, traits::node_container_size, constant_t<0>>::value;
            constexpr static auto out_edge_container_size = 0;

            constexpr static inline auto max_edge_compile_time = graph_traits::edge_container_size;
            constexpr static inline auto max_node_compile_time = node_container_size ?
                                                                 node_container_size :
                                                                 max_edge_compile_time * 2;

            template<class P, bool C, class = void>
            struct get_node_property_getter : type_identity<
                std::conditional_t<C, getter_t<1>, noop_t>> {};

            template<class P>
            struct get_node_property_getter<P, true, std::enable_if_t<
                std::is_same_v<get_property<P, node_property>,
                    typename graph_traits::node_repr_type
                >>> : type_identity<identity_t> {};

            constexpr static auto get_edge_property = std::conditional_t<graph_traits::has_edge_property, getter_t<2>, noop_t>{};
            constexpr static auto get_node_property = typename get_node_property_getter<Props, graph_traits::has_node_property>::type{};

            constexpr static inline auto edge_source = getter_t<0>{};
            constexpr static inline auto edge_target = getter_t<1>{};


            constexpr static inline auto get_edges = std::conditional_t<graph_traits::has_node_property, getter_t<1>,
                std::conditional_t<graph_traits::has_graph_property, getter_t<0>, identity_t>>{};
            constexpr static inline auto get_nodes = std::conditional_t<graph_traits::has_node_property, getter_t<0>, noop_t>{};

            using node_container_type = std::remove_reference_t<decltype(get_nodes(std::declval<T>()))>;
            using edge_container_type = std::remove_reference_t<decltype(get_edges(std::declval<T>()))>;
            using out_edge_container_type = void;
        };
    };

    struct graph : any_of<adjacency_list, adjacency_matrix, edge_list> {};

    template<class T>
    constexpr inline bool is_graph_v = graph::template is_valid<T>();

    template<class T>
    using graph_traits = typename graph::template graph_traits<T>;

    template<class T, bool = is_graph_v<T>>
    struct graph_sfinae_impl {};

    template<class T>
    struct graph_sfinae_impl<T, true> {
        using type = graph_traits<T>;
    };

    template<class T, class Why = decltype(graph::why_not<T>())>
    constexpr static bool is_it_a_graph = [] {
        if constexpr (std::is_void_v<Why>) {
            return true;
        } else {
            using simplified = typename Why::simplified;
            if constexpr (std::is_same_v<simplified, bxlx::not_matching_types>) {
                static_assert(why_not_graph<simplified>, "The type not matching any schema.");
            } else if constexpr (std::is_same_v<simplified, bxlx::graph_multiple_recognize>) {
                static_assert(why_not_graph<simplified>, "The type match more than one schema.");
            } else {
                static_assert(why_not_graph<simplified>, "On the schema, some property not matched.");
            }
            return false;
        }
    } ();
}

#endif //GRAPH_GRAPH_TRAITS_HPP
