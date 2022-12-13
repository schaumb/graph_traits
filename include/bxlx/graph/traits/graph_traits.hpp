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
            using simplified [[maybe_unused]] = not_matching_types;
        };
        template<class, class Properties>
        struct property_mismatch {
            using simplified [[maybe_unused]] = get_property_error<Properties>;
        };
        template<class, class...>
        struct multiple_graph_representation {
            using simplified [[maybe_unused]] = graph_multiple_recognize;
        };
        template<class>
        struct tuple_size_mismatch {
            using simplified [[maybe_unused]] = not_matching_types;
        };

        template<class ... All>
        struct any_nested_mismatched {
            using simplified [[maybe_unused]] = reduce_errors_t<typename All::simplified...>;
        };
        template<class ... All>
        struct tuple_nested_mismatched {
            using simplified [[maybe_unused]] = reduce_errors_t<typename All::simplified...>;
        };
        template<class, class Err>
        struct condition_and_error {
            using simplified [[maybe_unused]] = typename Err::simplified;
        };
        template<class C>
        struct condition_and_error<C, void> {
            using simplified [[maybe_unused]] = void;
        };
    }

    enum class graph_representation {
        adjacency_list,
        adjacency_matrix,
        edge_list,
    };

    enum class edge_direction_t {
        unknown,
        undirected [[maybe_unused]],
        directed [[maybe_unused]]
    };

    enum class allow_parallel_edges_t {
        unknown,
        yes [[maybe_unused]],
        no [[maybe_unused]]
    };

    template<detail2::type_classification ... types>
    struct accept_only {
        template<class T>
        constexpr static bool is_valid() {
            return ((detail2::classify<T> == types) || ...);
        }

        template<class T>
        constexpr static auto why_not() {
            if constexpr (!is_valid<T>()) {
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
    struct user_node_index {};
    struct user_edge_index {};
    struct node_repr_type {};
    struct edge_repr_type {};
    struct node_container_size {};
    struct edge_container_size {};
    struct inside_container_size {};
    struct node_equality_type {};
    struct node_comparator_type {};

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

    struct weaker_if_property_is_range {
        template<class T>
        using properties [[maybe_unused]] = std::conditional_t<
            detail2::is_range_v<T>,
            property<weaker_if_property_is_range, std::true_type>,
            empty_properties
        >;
    };

    struct edge_property : always_valid {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<
            get_properties<weaker_if_property_is_range, T>,
            property<edge_property, T>
        >;
    };
    struct node_property : always_valid {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<
            get_properties<weaker_if_property_is_range, T>,
            property<node_property, T>
        >;
    };
    struct graph_property : always_valid {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<
            get_properties<weaker_if_property_is_range, T>,
            property<graph_property, T>
        >;
    };

    struct node_index : accept_only<detail2::type_classification::indeterminate,
            detail2::type_classification::index, detail2::type_classification::optional> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<property<user_node_index, std::true_type>, property<node_index, std::remove_cv_t<T>>>;
    };

    struct edge_index : accept_only<detail2::type_classification::indeterminate,
        detail2::type_classification::index, detail2::type_classification::optional> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<property<user_edge_index, std::true_type>, property<edge_index, std::remove_cv_t<T>>>;
    };

    template<class index_type = node_index, class user_index_type = std::conditional_t<std::is_same_v<index_type, node_index>, user_node_index, user_edge_index>>
    struct index : accept_only<detail2::type_classification::index> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<property<user_index_type, std::false_type>, property<index_type, std::remove_cv_t<T>>>;
    };

    struct bool_t : accept_only<detail2::type_classification::bool_t>, no_prop {};
    struct bitset : accept_only<detail2::type_classification::bitset_like_container> {
        template<class T>
        using properties [[maybe_unused]] = merge_properties<
            std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0),
                property<node_container_size, constant_t<bxlx::detail2::compile_time_size_v<T>>>,
                empty_properties
            >,
            property<edge_repr_type, detail2::subscript_operator_return<T>>,
            property<bitset, std::true_type>
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
        constexpr static std::pair<std::size_t, bool> valid() {
            bool valid[sizeof...(Conditions)] {Conditions::template is_valid<T>() ...};
            bool has_weaker[sizeof...(Conditions)] {
                has_property_or_t<get_properties<std::conditional_t<
                    Conditions::template is_valid<T>(), Conditions, no_prop
                >, T>, weaker_if_property_is_range, std::false_type>::value...
            };
            bool valid_is_weak{}, multiple_valid_weak{};
            std::size_t res = ~std::size_t{};

            for (std::size_t i{}, s{sizeof...(Conditions)}; i != s; ++i) {
                if (!valid[i]) continue;
                if (bool weak = has_weaker[i]; res >= s || (valid_is_weak && !weak)) {
                    res = i;
                    valid_is_weak = weak;
                } else if (weak) {
                    multiple_valid_weak = true;
                } else {
                    return {~std::size_t{}, false};
                }
            }
            if (valid_is_weak && multiple_valid_weak)
                return {~std::size_t{}, false};

            return {res, res < sizeof...(Conditions)};
        }

        template<class T>
        constexpr static bool is_valid() {
            return valid<T>().second;
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
            return valid<T>().first;
        }

        template<class T>
        using properties [[maybe_unused]] = get_properties<std::tuple_element_t<the_valid_index<T>(), std::tuple<Conditions...>>, T>;

        template<class T>
        using graph_traits = typename std::tuple_element_t<the_valid_index<T>(), std::tuple<Conditions...>>::template graph_traits<T>;
    };

    template<class ...Conditions>
    struct tuple_like : accept_recursively<tuple_like<Conditions...>, detail2::type_classification::tuple_like> {
        template<class T, std::size_t...Ix>
        static merge_properties<empty_properties, get_properties<Conditions, detail2::tuple_element_cvref_t<Ix, T>>...>
        merge_properties_helper(std::index_sequence<Ix...>) { return {}; }

        template<class T>
        using properties = decltype(merge_properties_helper<T>(std::make_index_sequence<std::tuple_size_v<T>>{}));

        template<class T, std::size_t ...Ix>
        constexpr static bool is_valid_nested(std::index_sequence<Ix...>) {
            return (Conditions::template is_valid<detail2::tuple_element_cvref_t<Ix, T>>() && ...);
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
            return error_handling::tuple_nested_mismatched<error_handling::condition_and_error<Conditions, decltype(Conditions::template why_not<detail2::tuple_element_cvref_t<Ix, T>>())>...>{};
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
            >,
            std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0) && std::is_same_v<container_size_prop, inside_container_size>,
                property<user_node_index, std::false_type>,
                empty_properties
            >,
            std::conditional_t<!std::is_void_v<bxlx::detail2::range_member_traits::key_equal_t<T>> &&
                ((std::is_same_v<container_size_prop, inside_container_size> &&
                    (detail2::is_map_like_container_v<T> ||
                        std::is_void_v<has_property_or_t<get_properties<Cond, detail2::range_traits_type<T>>, edge_property, void>>)) ||
                 (std::is_same_v<container_size_prop, node_container_size> && detail2::is_map_like_container_v<T>)),
                property<node_equality_type, bxlx::detail2::range_member_traits::key_equal_t<T>>,
                empty_properties
            >,
            std::conditional_t<!std::is_void_v<bxlx::detail2::range_member_traits::key_compare_t<T>> &&
                ((std::is_same_v<container_size_prop, inside_container_size> &&
                    (detail2::is_map_like_container_v<T> ||
                        std::is_void_v<has_property_or_t<get_properties<Cond, detail2::range_traits_type<T>>, edge_property, void>>)) ||
                 (std::is_same_v<container_size_prop, node_container_size> && detail2::is_map_like_container_v<T>)),
                property<node_comparator_type, bxlx::detail2::range_member_traits::key_compare_t<T>>,
                empty_properties
            >
        >;
    };

    template<class Cond>
    struct range : accept_recursively<range_impl<inside_container_size, Cond>,
        detail2::type_classification::range,
        detail2::type_classification::sized_range,
        detail2::type_classification::random_access_range>, range_impl<inside_container_size, Cond> {
    };

    template<bool only_multimap, class Cond>
    struct edge_range : accept_recursively<edge_range<only_multimap, Cond>,
        detail2::type_classification::sized_range,
        detail2::type_classification::random_access_range>, range_impl<edge_container_size, Cond> {
        using Base = accept_recursively<edge_range<only_multimap, Cond>,
            detail2::type_classification::sized_range,
            detail2::type_classification::random_access_range>;

        template<class T>
        constexpr static bool is_valid() {
            if constexpr (!only_multimap || !detail2::is_map_like_container_v<T> || detail2::is_multi_v<T>) {
                return Base::template is_valid<T>();
            } else {
                return false;
            }
        }
    };

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
    template<class T, class ...Ts>
    struct with_node_property : with_property<node_property, T, Ts...> {
        template<class U>
        using properties [[maybe_unused]] = merge_properties<get_properties<with_property<node_property, T, Ts...>, U>,
            property<node_repr_type, U>>;
    };
    template<class T, class ...Ts>
    struct with_edge_property : with_property<edge_property, T, Ts...> {
        template<class U>
        using properties [[maybe_unused]] = merge_properties<get_properties<with_property<edge_property, T, Ts...>, U>,
            property<edge_repr_type, U>>;
    };

    template<class Property, class SizeProperty, class T, class U>
    struct map_like_range : accept_recursively<range_impl<SizeProperty, save_type<Property, tuple_like<T, U>>>,
        detail2::type_classification::sized_range, detail2::type_classification::random_access_range>,
        range_impl<SizeProperty, save_type<Property, tuple_like<T, U>>> {};

    template<class U>
    using node_map_save = map_like_range<node_repr_type, node_container_size, node_index, U>;


    template<class Ix>
    using edge_range_prop = any_of<
        edge_range<true, with_edge_property<Ix, Ix>>,
        edge_range<false, save_type<edge_repr_type, tuple_like<tuple_like<Ix, Ix>, edge_property>>>
    >;

    struct noop_t {
        template<class ...Ts>
        constexpr void operator()(Ts && ...) const noexcept {
        }
    };

    template<std::size_t I>
    struct getter_t {
        template<class T, class ...Ts>
        [[nodiscard]] constexpr auto operator()(T&& val, Ts&&...) const noexcept -> detail2::tuple_element_cvref_t<I, T> {
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


    template<auto>
    constexpr static inline identity_t pass{};

    template<class T, class With, std::size_t ...Ix>
    constexpr auto aggregate_initialize(std::index_sequence<Ix...>, const With& with) -> decltype(T{pass<Ix>(with)...}) {
        return T{pass<Ix>(with)...};
    }

    template<class, class, std::size_t, class = void>
    constexpr static bool is_aggregate_initializable_v = false;
    template<class T, class With, std::size_t N>
    constexpr static bool is_aggregate_initializable_v<T, With, N, std::void_t<
        decltype(aggregate_initialize<T, With>(std::make_index_sequence<N>{}, std::declval<const With&>()))
    >> = true;


    template<class Props>
    struct graph_traits_common {
        constexpr static bool has_graph_property = has_property<Props, graph_property>;
        constexpr static bool has_edge_property = has_property<Props, edge_property>;
        constexpr static bool has_node_property = has_property<Props, node_property>;
        using graph_property_type [[maybe_unused]] = has_property_or_t<Props, graph_property, void>;
        using node_property_type [[maybe_unused]] = has_property_or_t<Props, node_property, void>;
        using edge_property_type [[maybe_unused]] = has_property_or_t<Props, edge_property, void>;

        using node_index_t [[maybe_unused]] = has_property_or_t<Props, node_index, std::size_t>;
        constexpr static bool user_node_index = get_property<Props, traits::user_node_index>::value;
        using edge_index_t [[maybe_unused]] = has_property_or_t<Props, edge_index, void>;
        constexpr static bool user_edge_index = has_property_or_t<Props, traits::user_edge_index, std::false_type>::value;

        using node_repr_type [[maybe_unused]] = has_property_or_t<Props, traits::node_repr_type, void>;
        using edge_repr_type [[maybe_unused]] = get_property<Props, edge_repr_type>;

        using node_equality_type [[maybe_unused]] = std::conditional_t<
            user_node_index,
            has_property_or_t<Props, node_equality_type, void>,
            std::equal_to<node_index_t>
        >;

        using node_comparator_type [[maybe_unused]] = std::conditional_t<
            user_node_index,
            has_property_or_t<Props, node_comparator_type, void>,
            std::less<node_index_t>
        >;

        [[maybe_unused]] constexpr static edge_direction_t edge_direction =
            has_property_or_t<Props, edge_direction_t, constant_t<edge_direction_t::unknown>>::value;
        [[maybe_unused]] constexpr static allow_parallel_edges_t allow_parallel_edges =
            has_property_or_t<Props, allow_parallel_edges_t, constant_t<allow_parallel_edges_t::unknown>>::value;
    };

    struct adjacency_list : with_graph_property<any_of<
        node_indexed_range<with_node_property<
            range<with_edge_property<index<>>>
        >>,
        node_map_save<with_property<node_property,
            range<with_edge_property<node_index>>
        >>
    >> {
        template<class T, class Props = get_properties<adjacency_list, T>>
        struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {
            constexpr static inline auto representation = graph_representation::adjacency_list;
            using node_index_t = typename graph_traits::node_index_t;

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
                    detail2::tuple_element_cvref_t<1, get_property<P, traits::node_repr_type>>
            >>> : type_identity<getter_t<1>> {};

            constexpr static auto get_edge_property = std::conditional_t<graph_traits::has_edge_property, getter_t<1>, noop_t>{};
            constexpr static auto get_node_property = typename get_node_property_getter<Props, graph_traits::has_node_property>::type{};

            constexpr static inline auto edge_target = std::conditional_t<graph_traits::has_edge_property, getter_t<0>, identity_t>{};
            constexpr static inline auto out_edges = composition_t<
                std::conditional_t<graph_traits::has_node_property, getter_t<0>, identity_t>,
                std::conditional_t<graph_traits::user_node_index, getter_t<1>, identity_t>>{};

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
            using edge_container_type [[maybe_unused]] = void;
            using out_edge_container_type = std::remove_reference_t<decltype(out_edges(std::declval<typename graph_traits::node_repr_type>()))>;


            constexpr static auto out_edges_initializer = [] () -> decltype(auto) {
                if constexpr (out_edge_container_size != 0 && !graph_traits::has_edge_property &&
                              is_aggregate_initializable_v<out_edge_container_type, const node_index_t&, out_edge_container_size>) {
                    return std::tuple{aggregate_initialize<out_edge_container_type>(
                        std::make_index_sequence<out_edge_container_size>{}, invalid
                    )};
                } else if constexpr (out_edge_container_size != 0 && graph_traits::has_edge_property &&
                    is_aggregate_initializable_v<out_edge_container_type,
                        typename graph_traits::edge_repr_type, out_edge_container_size>) {
                    return std::tuple{aggregate_initialize<out_edge_container_type>(
                        std::make_index_sequence<out_edge_container_size>{},
                        typename graph_traits::edge_repr_type{invalid, {}}
                    )};
                } else {
                    return std::tuple{};
                }
            };

            constexpr static auto after_add_node = [](auto& new_node) {
                if constexpr (out_edge_container_size != 0 &&
                              std::tuple_size_v<decltype(out_edges_initializer())> == 0) {
                    if constexpr (!std::is_const_v<std::remove_reference_t<
                        decltype(edge_target(*std::begin(out_edges(new_node))))
                    >>) {
                        for (auto& edge : out_edges(new_node)) {
                            edge_target(edge) = invalid;
                        }
                    } else {
                        return std::false_type{};
                    }
                } else {
                    return std::true_type{};
                }
            };

            template<class traits = graph_traits, class ...Args>
            constexpr static inline auto add_node(std::enable_if_t<!traits::user_node_index, T>& g, Args&& ...args) {
                constexpr bool no_arg = sizeof...(args) == 0;
                constexpr bool overridable_if_needed = decltype(after_add_node(std::declval<node_repr_type&>()))::value;
                auto& nodes = get_nodes(g);
                node_index_t ix = std::size(nodes);

                if constexpr (traits::has_node_property &&
                    detail2::range_member_traits::has_emplace_back_v<node_container_type,
                        std::piecewise_construct_t,
                        decltype(out_edges_initializer()),
                        std::tuple<Args&&...>
                    > && overridable_if_needed
                ) {
                    if constexpr (std::is_constructible_v<typename traits::node_property_type,
                        Args&&...
                    >) {
                        after_add_node(nodes.emplace_back(std::piecewise_construct,
                                                          out_edges_initializer(), // tuple<> or tuple<T>(T{invalid...});
                                                          std::forward_as_tuple(std::forward<Args&&>(args)...)
                        ));
                        return ix;
                    }
                } else if constexpr (!traits::has_node_property && no_arg && (out_edge_container_size == 0 ||
                                     std::tuple_size_v<decltype(out_edges_initializer())> == 0) &&
                    detail2::range_member_traits::has_emplace_back_v<node_container_type> && overridable_if_needed
                ) {
                    after_add_node(nodes.emplace_back());
                    return ix;
                } else if constexpr (!traits::has_node_property && no_arg && std::tuple_size_v<decltype(out_edges_initializer())> > 0
                ) {
                    if constexpr (
                        detail2::range_member_traits::has_emplace_back_v<node_container_type,
                            detail2::tuple_element_cvref_t<0, decltype(out_edges_initializer())>
                        >
                    ) {
                        after_add_node(nodes.emplace_back(std::get<0>(out_edges_initializer())));
                        return ix;
                    }
                }
            };

            template<class traits = graph_traits, class ...Args>
            constexpr static inline auto add_node(std::enable_if_t<traits::user_node_index, T>& g, const node_index_t& i, Args&& ...args) {
                constexpr bool no_arg = sizeof...(args) == 0;
                if constexpr (traits::has_node_property &&
                    detail2::range_member_traits::has_try_emplace_v<
                        node_container_type,
                        const node_index_t&,
                        std::piecewise_construct_t,
                        std::tuple<>,
                        std::tuple<Args&&...>
                    >
                ) {
                    if constexpr (std::is_constructible_v<typename traits::node_property_type,
                        Args&&...
                    >) {
                        return std::make_pair(i, get_nodes(g).try_emplace(i,
                                                                          std::piecewise_construct,
                                                                          std::tuple<>(),
                                                                          std::forward_as_tuple(
                                                                              std::forward<Args &&>(args)...)
                        ).second);
                    }
                } else if constexpr (!traits::has_node_property && no_arg &&
                    detail2::range_member_traits::has_try_emplace_v<
                        node_container_type,
                        const node_index_t&
                    >
                ) {
                    return std::make_pair(i, get_nodes(g).try_emplace(i).second);
                }
            };

            template<class, class ...>
            constexpr static inline bool can_add_node_impl = false;
            template<class ...Args>
            constexpr static inline bool can_add_node_impl<std::void_t<
                decltype(add_node<>(std::declval<T&>(), std::declval<Args>()...))
            >, Args...> = !std::is_void_v<decltype(add_node<>(std::declval<T&>(), std::declval<Args>()...))>;

            template<class ...Args>
            constexpr static inline bool can_add_node = can_add_node_impl<void, Args...>;
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
            [[maybe_unused]] constexpr static inline auto is_bitset = has_property_or_t<Props, bitset, std::false_type>::value;

            using node_index_t = typename graph_traits::node_index_t;

            constexpr static inline auto out_edges = std::conditional_t<graph_traits::has_node_property, getter_t<0>, identity_t>{};

            constexpr static inline auto get_edges = noop_t{};
            constexpr static inline auto get_nodes = std::conditional_t<graph_traits::has_graph_property, getter_t<0>, identity_t>{};

            using node_container_type = std::remove_reference_t<decltype(get_nodes(std::declval<T>()))>;
            using edge_container_type [[maybe_unused]] = void;
            using out_edge_container_type = std::remove_reference_t<decltype(out_edges(std::declval<typename graph_traits::node_repr_type>()))>;

            constexpr static auto node_container_size = detail2::compile_time_size_v<node_container_type>;
            constexpr static auto out_edge_container_size = detail2::compile_time_size_v<out_edge_container_type>;

            constexpr static inline auto max_node_compile_time = std::max(node_container_size, out_edge_container_size);
            constexpr static inline auto max_edge_compile_time = max_node_compile_time * max_node_compile_time;

            constexpr static auto get_edge_property = std::conditional_t<graph_traits::has_edge_property, indirect_t, noop_t>{};
            constexpr static auto get_node_property = std::conditional_t<graph_traits::has_node_property, getter_t<1>, noop_t>{};


            constexpr static auto after_add_node = [] (auto& nodes) {
                if constexpr (out_edge_container_size == 0) {
                    if constexpr (
                        detail2::range_member_traits::has_resize_v<
                            out_edge_container_type
                        >
                    ) {
                        node_index_t size = std::size(nodes);
                        for (auto& node : nodes) {
                            out_edges(node).resize(size);
                        }
                        return std::true_type{};
                    } else {
                        return std::false_type{};
                    }
                } else {
                    return std::true_type{};
                }
            };

            template<class traits = graph_traits, class ...Args>
            constexpr static auto add_node (std::enable_if_t<!traits::user_node_index, T>& g, Args&& ...args) {
                constexpr bool no_arg = sizeof...(args) == 0;
                auto& nodes = get_nodes(g);
                node_index_t ix = std::size(nodes);
                constexpr bool resizable_if_needed = decltype(after_add_node(nodes))::value;

                if constexpr (traits::has_node_property &&
                    detail2::range_member_traits::has_emplace_back_v<
                        node_container_type,
                        std::piecewise_construct_t,
                        std::tuple<>,
                        std::tuple<Args&&...>
                    > && resizable_if_needed
                ) {
                    if constexpr (std::is_constructible_v<typename traits::node_property_type,
                        Args&&...
                    >) {
                        nodes.emplace_back(std::piecewise_construct,
                                           std::tuple<>{},
                                           std::forward_as_tuple(std::forward<Args &&>(args)...)
                        );
                        after_add_node(nodes);
                        return ix;
                    }
                } else if constexpr (!traits::has_node_property && no_arg &&
                    detail2::range_member_traits::has_emplace_back_v<
                        node_container_type
                    > && resizable_if_needed
                ) {
                    nodes.emplace_back();
                    after_add_node(nodes);
                    return ix;
                }
            };

            template<class traits = graph_traits, class ...Args>
            constexpr static auto add_node(std::enable_if_t<traits::user_node_index, T>& g, const node_index_t& i, Args&& ...args) {
                constexpr bool no_arg = sizeof...(args) == 0;
                auto& nodes = get_nodes(g);
                constexpr bool resizable_if_needed = decltype(after_add_node(nodes))::value;

                if constexpr (traits::has_node_property &&
                    detail2::range_member_traits::has_try_emplace_v<
                        node_container_type,
                        const node_index_t&,
                        std::piecewise_construct_t,
                        std::tuple<>,
                        std::tuple<Args&&...>
                    > && resizable_if_needed
                ) {
                    if constexpr (std::is_constructible_v<typename traits::node_property_type,
                        Args&&...
                    >) {
                        auto res = std::make_pair(i, nodes.try_emplace(i,
                                                                       std::piecewise_construct,
                                                                       std::tuple<>(),
                                                                       std::forward_as_tuple(
                                                                           std::forward<Args &&>(args)...)
                        ).second);
                        after_add_node(nodes);
                        return res;
                    }
                } else if constexpr (!traits::has_node_property && no_arg &&
                    detail2::range_member_traits::has_try_emplace_v<
                        node_container_type,
                        const node_index_t&
                    > && resizable_if_needed
                ) {
                    auto res = std::make_pair(i, nodes.try_emplace(i).second);
                    after_add_node(nodes);
                    return res;
                }
            };

            template<class, class ...>
            constexpr static inline bool can_add_node_impl = false;
            template<class ...Args>
            constexpr static inline bool can_add_node_impl<std::void_t<
                decltype(add_node<>(std::declval<T&>(), std::declval<Args>()...))
            >, Args...> = !std::is_void_v<decltype(add_node<>(std::declval<T&>(), std::declval<Args>()...))>;

            template<class ...Args>
            constexpr static inline bool can_add_node = can_add_node_impl<void, Args...>;
        };
    };

    struct edge_list : any_of<
        with_graph_property<edge_range_prop<node_index>>,
        with_graph_property<node_map_save<node_property>, edge_range_prop<node_index>>,
        with_graph_property<node_indexed_range<save_type<node_repr_type, node_property>>, edge_range_prop<index<>>>
    > {
        template<class T, class Props = get_properties<edge_list, T>>
        struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {
            constexpr static inline auto representation = graph_representation::edge_list;
            using node_index_t = typename graph_traits::node_index_t;

            constexpr static auto node_container_size = has_property_or_t<Props, traits::node_container_size, constant_t<0>>::value;
            constexpr static auto out_edge_container_size = 0;

            constexpr static inline auto max_edge_compile_time = has_property_or_t<Props, traits::edge_container_size, constant_t<0>>::value;;
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
            using out_edge_container_type [[maybe_unused]] = void;

            template<class traits = graph_traits, class ...Args>
            constexpr static auto add_node(std::enable_if_t<!traits::user_node_index, T>& g, Args&& ...args) {
                if constexpr (!std::is_void_v<node_container_type> &&
                    detail2::range_member_traits::has_emplace_back_v<
                        node_container_type,
                        Args&&...
                    > &&
                    std::is_constructible_v<typename traits::node_property_type,
                        Args&&...
                    >
                ) {
                    auto &nodes = get_nodes(g);
                    node_index_t ix = std::size(nodes);
                    nodes.emplace_back(std::forward<Args &&>(args)...);
                    return ix;
                }
            };

            template<class traits = graph_traits, class ...Args>
            constexpr static auto add_node(std::enable_if_t<traits::user_node_index, T>& g, const node_index_t& i, Args&& ...args) {
                if constexpr (!std::is_void_v<node_container_type> &&
                    detail2::range_member_traits::has_try_emplace_v<
                        node_container_type,
                        const node_index_t&,
                        Args&&...
                    > &&
                    std::is_constructible_v<typename traits::node_property_type,
                        Args&&...
                    >
                ) {
                    return std::make_pair(i, get_nodes(g).try_emplace(i, std::forward<decltype(args)>(args)...).second);
                }
            };

            template<class, class ...>
            constexpr static inline bool can_add_node_impl = false;
            template<class ...Args>
            constexpr static inline bool can_add_node_impl<std::void_t<
                decltype(add_node<>(std::declval<T&>(), std::declval<Args>()...))
            >, Args...> = !std::is_void_v<decltype(add_node<>(std::declval<T&>(), std::declval<Args>()...))>;

            template<class ...Args>
            constexpr static inline bool can_add_node = can_add_node_impl<void, Args...>;
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
        using type [[maybe_unused]] = graph_traits<T>;
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
