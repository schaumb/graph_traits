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


    enum class graph_representation_t {
        unknown,
        adjacency_list,
        adjacency_matrix,
        edge_list,
    };

    enum class edge_direction_t {
        unknown,
        undirected,
        directed,
        bidirectional,
        runtime
    };

    enum class allow_parallel_edges_t {
        unknown,
        yes,
        no
    };

    namespace traits {
        template<auto p>
        using constant_t = std::integral_constant<decltype(p), p>;

        template<class T>
        struct type_identity { using type = T; };

        template<class T>
        using t_ = typename T::type;
    }

    constexpr static inline traits::constant_t<edge_direction_t::undirected> undirected{};
    constexpr static inline traits::constant_t<edge_direction_t::directed> directed{};
    constexpr static inline traits::constant_t<edge_direction_t::bidirectional> bidirectional{};

    constexpr static inline traits::constant_t<allow_parallel_edges_t::yes> allow_parallel_edges{};
    constexpr static inline traits::constant_t<allow_parallel_edges_t::no> disallow_parallel_edges{};

    namespace properties_impl {
        template<class T, class V = std::make_index_sequence<std::tuple_size_v<T>>>
        struct properties;
        template<class T, std::size_t ...ix>
        struct properties<T, std::index_sequence<ix...>> {
            using type [[maybe_unused]] = T;
            template<class Key, class Prop>
            constexpr static inline bool is_this_property = std::is_same_v<Key, std::tuple_element_t<0, Prop>>;
            template<class Key>
            constexpr static inline bool has_property = (is_this_property<Key, std::tuple_element_t<ix, T>> || ...);

            template<class Prop1, class Prop2>
            constexpr static inline bool is_not_contradict_properties =
                !is_this_property<std::tuple_element_t<0, Prop1>, Prop2> ||
                std::is_same_v<std::tuple_element_t<1, Prop1>, std::tuple_element_t<1, Prop2>>;
            template<class Prop>
            constexpr static inline bool is_valid_prop = (is_not_contradict_properties<Prop, std::tuple_element_t<ix, T>> && ...);
            constexpr static inline bool is_valid = (is_valid_prop<std::tuple_element_t<ix, T>> && ...);

            template<class Key>
            constexpr static inline std::size_t get_property_index =
                std::min({(is_this_property<Key, std::tuple_element_t<ix, T>> ? ix : ~std::size_t{})...});

            template<class Key>
            constexpr static inline std::size_t get_last_property_index =
                std::max({(is_this_property<Key, std::tuple_element_t<ix, T>> ? ix : std::size_t{})...});

            constexpr static inline std::size_t get_first_invalid_prop_index =
                std::min({(!is_valid_prop<std::tuple_element_t<ix, T>> ? ix : ~std::size_t{})...});
        };
        using traits::t_;

        using empty = properties<std::tuple<>>;
        template<class K, class U>
        using prop = properties<std::tuple<std::pair<K, U>>>;
        template<auto p>
        using constant = prop<decltype(p), traits::constant_t<p>>;
        template<class K, auto p>
        using constant_prop = prop<K, traits::constant_t<p>>;

        template<class ...Properties>
        using merge_properties = properties<decltype(std::tuple_cat(std::declval<t_<Properties>>()...))>;

        template<class Properties, class Key>
        constexpr static bool has_property = Properties::template has_property<Key>;
        template<class Properties, class Key>
        using get_property = std::tuple_element_t<1, std::tuple_element_t<Properties::template get_property_index<Key>, t_<Properties>>>;
        template<class Properties,
            class InvKey = std::tuple_element_t<0, std::tuple_element_t<Properties::get_first_invalid_prop_index, t_<Properties>>>>
        using get_property_error = bxlx::multiple_property_for<InvKey, get_property<Properties, InvKey>,
            std::tuple_element_t<1, std::tuple_element_t<Properties::template get_last_property_index<InvKey>, t_<Properties>>>>;

        template<class Properties, class Key, class Default, class = void>
        struct has_property_or : traits::type_identity<Default> {};
        template<class Properties, class Key, class Default>
        struct has_property_or<Properties, Key, Default, std::enable_if_t<
            has_property<Properties, Key>
        >> : traits::type_identity<get_property<Properties, Key>> {};
        template<class Properties, class Key, class Default = void>
        using has_property_or_t = t_<has_property_or<Properties, Key, Default>>;

        template<class Type, class U>
        using get_properties = typename Type::template properties<U>;
    }

    namespace error_handling {
        using traits::t_;
        using traits::type_identity;

        template<class Error, class...>
        struct reduce_errors : type_identity<Error> {};

        template<class Error1, class Error2, class... OtherErrors>
        struct reduce_errors<Error1, Error2, OtherErrors...> :
            reduce_errors<std::conditional_t<std::is_void_v<Error1> || std::is_same_v<Error1, not_matching_types> ||
                                             std::is_same_v<Error2, graph_multiple_recognize>, Error2, Error1>, OtherErrors...> {};

        template<class ...Ts>
        using reduce_errors_t = t_<reduce_errors<void, Ts...>>;

        // detailed errors
        template<class>
        struct type_classification_mismatch {
            using simplified [[maybe_unused]] = not_matching_types;
        };
        template<class, class Properties>
        struct property_mismatch {
            using simplified [[maybe_unused]] = properties_impl::get_property_error<Properties>;
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
        template<class, class Err, class Props = void>
        struct condition_and_error {
            using simplified [[maybe_unused]] = typename Err::simplified;
        };
        template<class C, class Props>
        struct condition_and_error<C, void, Props> {
            using simplified [[maybe_unused]] = void;
        };

        template<class C, class, class = void>
        struct nested_condition : type_identity<C> {};
        template<class C, class T>
        struct nested_condition<C, T, std::void_t<
            typename C::template nested_condition<T>
        >> : type_identity<typename C::template nested_condition<T>> {};
        template<class C, class T>
        using nested_condition_t = typename nested_condition<C, T>::type;
    }

    namespace traits {
        namespace prop = properties_impl;
        using prop::get_property;
        using prop::get_properties;
        using prop::merge_properties;
        using prop::has_property;
        using prop::has_property_or_t;

        namespace keys {
            enum EdgeNode {
                edge,
                node
            };

            // properties
            template<EdgeNode> struct is_user_index;
            template<EdgeNode> struct index_type;
            template<EdgeNode> struct container;
            template<EdgeNode> struct property;

            using is_user_node_index = is_user_index<node>;
            using is_user_edge_index = is_user_index<edge>;
            using node_index_type = index_type<node>;
            using edge_index_type = index_type<edge>;
            using node_container = container<node>;
            using edge_container = container<edge>;
            using node_property = property<node>;
            using edge_property = property<edge>;

            struct in_edge_list_container;
            struct out_edge_list_container;
            struct edge_list_container;

            template<class containerKey>
            struct container_size;

            using node_container_size = container_size<node_container>;
            using edge_container_size = container_size<edge_container>;
            using in_edge_list_container_size = container_size<in_edge_list_container>;
            using out_edge_list_container_size = container_size<out_edge_list_container>;
            using edge_list_container_size = container_size<edge_list_container>;

            struct graph_property;
        };

        struct no_prop {
            template<class>
            using properties [[maybe_unused]] = prop::empty;
        };

        struct always_valid : no_prop {
            template<class>
            constexpr static bool is_valid() {
                return true;
            }

            template<class>
            constexpr static void why_not() {}
        };

        template<detail2::type_classification ... types>
        struct accept_only : no_prop {
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
        struct accept_recursively : no_prop {
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

        struct weaker_if_property_is_range : always_valid {
            template<class T>
            using properties [[maybe_unused]] = std::conditional_t<
                detail2::is_range_v<T>,
                prop::constant_prop<weaker_if_property_is_range, true>,
                prop::empty
            >;
        };

        template<class KeyType, class Nested>
        struct save_type : Nested {
            template<class U>
            using properties [[maybe_unused]] = merge_properties<get_properties<Nested, U>,
                prop::prop<KeyType, U>>;
        };

        template<class key_t>
        using graph_save_property = save_type<key_t, weaker_if_property_is_range>;
        template<keys::EdgeNode en>
        using en_property = graph_save_property<keys::property<en>>;
        using edge_property = en_property<keys::edge>;
        using node_property = en_property<keys::node>;
        using graph_property = graph_save_property<keys::graph_property>;

        template<keys::EdgeNode en, bool user_defined>
        struct index : std::conditional_t<user_defined,
            accept_only<detail2::type_classification::indeterminate,
                        detail2::type_classification::index,
                        detail2::type_classification::optional>,
            accept_only<detail2::type_classification::index>>
        {
            template<class T>
            using properties [[maybe_unused]] = merge_properties<
                prop::constant_prop<keys::is_user_index<en>, user_defined>,
                prop::prop<keys::index_type<en>, std::remove_cv_t<T>>
            >;
        };
        using node_index = index<keys::node, false>;
        using node_key = index<keys::node, true>;
        using edge_index = index<keys::edge, false>;
        using edge_key = index<keys::edge, true>;

        using bool_t = accept_only<detail2::type_classification::bool_t>;
        using edge_count = accept_only<detail2::type_classification::index>;

        template<class container_size_prop>
        struct bitset : accept_only<detail2::type_classification::bitset_like_container> {
            template<class T>
            using properties [[maybe_unused]] =
                std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0),
                    prop::prop<container_size_prop, constant_t<bxlx::detail2::compile_time_size_v<T>>>,
                    prop::empty
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

            template<class T>
            using nested_condition [[maybe_unused]] =
                optional<error_handling::nested_condition_t<Cond, detail2::optional_traits_type<T>>>;
        };


        template<class... Conditions>
        struct any_of {
            constexpr static inline auto conditions = sizeof...(Conditions);
            static_assert(conditions != 0);
            template<class T>
            constexpr static std::pair<std::size_t, bool> valid() {
                bool valid[conditions] {Conditions::template is_valid<T>() ...};
                bool has_weaker[conditions] {
                    prop::has_property_or_t<get_properties<std::conditional_t<
                        Conditions::template is_valid<T>(), Conditions, no_prop
                    >, T>, weaker_if_property_is_range, std::false_type>::value...
                };
                bool valid_is_weak{}, multiple_valid_weak{};
                std::size_t res = ~std::size_t{};

                for (std::size_t i{}; i != conditions; ++i) {
                    if (!valid[i]) continue;
                    if (bool weak = has_weaker[i]; res >= conditions || (valid_is_weak && !weak)) {
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

                return {res, res < conditions};
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
                            error_handling::condition_and_error<
                                typename std::conditional_t<
                                    Conditions::template is_valid<T>(),
                                    error_handling::nested_condition<Conditions, T>,
                                    type_identity<Conditions>
                                >::type,
                                constant_t<Conditions::template is_valid<T>()>,
                                get_properties<std::conditional_t<
                                    Conditions::template is_valid<T>(), Conditions, no_prop
                                >, T>
                                >...>{};
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
            using nested_condition [[maybe_unused]] = std::tuple_element_t<the_valid_index<T>(), std::tuple<Conditions...>>;

            template<class T>
            using properties [[maybe_unused]] = get_properties<nested_condition<T>, T>;

            template<class T>
            using graph_traits = typename std::tuple_element_t<the_valid_index<T>(), std::tuple<Conditions...>>::template graph_traits<T>;
        };

        template<class Indices, class ...Conditions>
        struct tuple_like_impl;
        template<std::size_t ...Ix, class ...Conditions>
        struct tuple_like_impl<std::index_sequence<Ix...>, Conditions...> : accept_recursively<tuple_like_impl<std::index_sequence<Ix...>, Conditions...>, detail2::type_classification::tuple_like> {
            template<class T>
            using properties = merge_properties<get_properties<Conditions, detail2::tuple_element_cvref_t<Ix, T>>...>;

            template<class T>
            constexpr static bool is_valid_nested() {
                if constexpr (sizeof...(Conditions) == std::tuple_size_v<T>) {
                    if constexpr ((Conditions::template is_valid<detail2::tuple_element_cvref_t<Ix, T>>() && ...)) {
                        return properties<T>::is_valid;
                    }
                }
                return false;
            }

            template<class T>
            constexpr static auto why_not_nested() {
                if constexpr (sizeof...(Conditions) == std::tuple_size_v<T>) {
                    if constexpr ((Conditions::template is_valid<detail2::tuple_element_cvref_t<Ix, T>>() && ...)) {
                        return error_handling::property_mismatch<T, get_properties<tuple_like_impl, T>>{};
                    } else {
                        return error_handling::tuple_nested_mismatched<error_handling::condition_and_error<Conditions,
                            decltype(Conditions::template why_not<detail2::tuple_element_cvref_t<Ix, T>>())>...>{};
                    }
                } else {
                    return error_handling::tuple_size_mismatch<T>{};
                }
            }

            template<class T>
            using nested_condition [[maybe_unused]] = tuple_like_impl<std::index_sequence<Ix...>, error_handling::nested_condition_t<Conditions, detail2::tuple_element_cvref_t<Ix, T>>...>;
        };

        template<class ...Conditions>
        using tuple_like = tuple_like_impl<std::index_sequence_for<Conditions...>, Conditions...>;

        template<class Cond,
            class container_prop,
            class container_size_prop = keys::container_size<container_prop>,
            class OtherProperties = prop::empty,
            auto* Checker = (void*)nullptr>
        struct range_impl : accept_recursively<range_impl<Cond, container_prop,
            container_size_prop, OtherProperties, Checker>, detail2::type_classification::range> {
            template<class T>
            constexpr static bool is_valid_nested() {
                if constexpr (Checker != nullptr) {
                    if constexpr (!(*Checker)(type_identity<T>{})) {
                        return false;
                    }
                }
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
                OtherProperties,
                prop::prop<container_prop, T>,
                // if it has size, save
                std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0),
                    prop::prop<container_size_prop, constant_t<bxlx::detail2::compile_time_size_v<T>>>,
                    prop::empty
                >
                /*,
                // parallel edge property
                std::conditional_t<detail2::is_associative_container_v<T> &&
                        (std::is_same_v<container_size_prop, adjacency_container_size> ||
                         std::is_same_v<container_size_prop, edge_list_container_size>),
                    constant_p<detail2::is_multi_v<T> ? allow_parallel_edges_t::yes : allow_parallel_edges_t::no>,
                    empty_properties
                >,
                // prohibited user defined node index if adjacency container size is compile time known and not associative
                std::conditional_t<(bxlx::detail2::compile_time_size_v<T> > 0) &&
                        std::is_same_v<container_size_prop, adjacency_container_size> &&
                        !detail2::is_associative_container_v<T>,
                    property<is_user_node_index, std::false_type>,
                    empty_properties
                >,
                // save repr type if passed
                std::conditional_t<std::is_void_v<save_repr_type_as>,
                    empty_properties,
                    property<save_repr_type_as, detail2::range_traits_type<T>>
                >*/
            >;

            template<class T>
            using nested_condition [[maybe_unused]] = range_impl<
                error_handling::nested_condition_t<Cond, detail2::range_traits_type<T>>,
                container_prop, container_size_prop, OtherProperties, Checker>;
        };

        template<class Property, class T, class ...Ts>
        using with_property = any_of<tuple_like<T, Ts..., Property>,
            std::conditional_t<sizeof...(Ts) == 0, T, tuple_like<T, Ts...>>>;

        template<class T, class ...Ts>
        using with_graph_property = with_property<graph_property, T, Ts...>;
        template<keys::EdgeNode en, class T, class ...Ts>
        using with_en_property = with_property<en_property<en>, T, Ts...>;
        template<class T, class ...Ts>
        using with_edge_property = with_en_property<keys::edge, T, Ts...>;
        template<class T, class ...Ts>
        using with_node_property = with_en_property<keys::node, T, Ts...>;

        constexpr auto only_random_access_range = [](auto t) {
            return detail2::is_random_access_range_v<typename decltype(t)::type>;
        };

        template<keys::EdgeNode en, class Nested, class container_prop = keys::container<en>,
            class container_size_prop = keys::container_size<container_prop>>
        using indexed_range = range_impl<
                Nested,
            container_prop, container_size_prop,
            prop::constant_prop<keys::is_user_index<en>, false>,
            &only_random_access_range>;

        template<class Nested = node_property, class container_prop = keys::container<keys::node>,
                class container_size_prop = keys::container_size<container_prop>>
        using node_indexed_range = indexed_range<keys::node,
            std::conditional_t<std::is_same_v<Nested, node_property>, node_property, with_node_property<Nested>>,
            container_prop, container_size_prop>;

        template<class Nested = edge_property, class container_prop = keys::container<keys::edge>,
            class container_size_prop = keys::container_size<container_prop>>
        using edge_indexed_range = indexed_range<keys::edge,
            std::conditional_t<std::is_same_v<Nested, edge_property>, edge_property, with_edge_property<Nested>>,
            container_prop, container_size_prop>;

        template<keys::EdgeNode en, class Nested,
            class container_prop = keys::container<en>,
            class container_size_prop = keys::container_size<container_prop>>
        using map_range = range_impl<tuple_like<index<en, true>,
            Nested>, container_prop, container_size_prop>;

        template<class Nested = node_property ,
            class container_prop = keys::container<keys::node>,
            class container_size_prop = keys::container_size<container_prop>>
        using node_map_range = map_range<keys::node,
            std::conditional_t<std::is_same_v<Nested, node_property>, node_property, with_node_property<Nested>>
        , container_prop, container_size_prop>;

        template<class Nested = edge_property,
            class container_prop = keys::container<keys::edge>,
            class container_size_prop = keys::container_size<container_prop>>
        using edge_map_range = map_range<keys::edge,
            std::conditional_t<std::is_same_v<Nested, edge_property>, edge_property, with_edge_property<Nested>>
        , container_prop, container_size_prop>;

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
            constexpr static bool has_graph_property = has_property<Props, keys::graph_property>;
            constexpr static bool has_edge_property = has_property<Props, keys::edge_property>;
            constexpr static bool has_node_property = has_property<Props, keys::node_property>;
            using graph_property_type [[maybe_unused]] = has_property_or_t<Props, keys::graph_property, void>;
            using edge_property_type [[maybe_unused]] = has_property_or_t<Props, keys::edge_property, void>;
            using node_property_type [[maybe_unused]] = has_property_or_t<Props, keys::node_property, void>;

            using node_index_t [[maybe_unused]] = has_property_or_t<Props, keys::node_index_type, std::size_t>;
            constexpr static bool user_node_index = get_property<Props, keys::is_user_node_index>::value;
            using edge_index_t [[maybe_unused]] = has_property_or_t<Props, keys::edge_index_type, void>;
            constexpr static bool user_edge_index = has_property_or_t<Props, keys::is_user_edge_index, std::false_type>::value;

            [[maybe_unused]] constexpr static edge_direction_t edge_direction =
                has_property_or_t<Props, edge_direction_t, constant_t<edge_direction_t::unknown>>::value;
            [[maybe_unused]] constexpr static allow_parallel_edges_t allow_parallel_edges =
                has_property_or_t<Props, allow_parallel_edges_t, constant_t<allow_parallel_edges_t::unknown>>::value;
        };

        template<template<class> class EdgePropertySeparator, template<class> class ... Others>
        using edge_index_as = any_of<
            with_graph_property<
                EdgePropertySeparator<edge_property>,
                Others<edge_property>...
            >,
            with_graph_property<
                EdgePropertySeparator<edge_index>,
                Others<edge_index>...,
                edge_indexed_range<>
            >,
            with_graph_property<
                EdgePropertySeparator<edge_key>,
                Others<edge_key>...,
                edge_map_range<>
            >
        >;

        template<class node_index_t, class edge_repr_t,
            class container_key = keys::out_edge_list_container>
        struct adj_range : range_impl<
            any_of<
                tuple_like<
                    node_index_t,
                    edge_repr_t
                >,
                node_index_t
            >,
            container_key
        > {};

        struct queue_partition_point_type;
        struct partition_point : save_type<queue_partition_point_type,
                                        accept_only<detail2::type_classification::optional,
                                                    detail2::type_classification::index,
                                                    detail2::type_classification::indeterminate>> {};

        template<class node_index_t, class edge_repr_t,
            class container_key = keys::out_edge_list_container>
        struct adj_range_if : adj_range<node_index_t, edge_repr_t, container_key> {};

        template<class node_index_t, class container_key>
        struct adj_range_if<node_index_t, edge_property, container_key> :
            range_impl<node_index_t, container_key>
        {};


        template<class node_index_t, class edge_repr_t>
        struct partitioned_queue : adj_range_if<node_index_t, edge_repr_t, keys::edge_list_container> {
            using base = adj_range_if<node_index_t, edge_repr_t, keys::edge_list_container>;
            template<class T>
            constexpr static bool is_valid() {
                if constexpr (base::template is_valid<T>()) {
                    return detail2::is_queue_like_container_v<T>;
                }
                return false;
            }

            template<class T>
            constexpr static auto why_not() {
                if constexpr (!base::template is_valid<T>()) {
                    return base::template why_not<T>();
                } else if constexpr (!is_valid<T>()) {
                    return error_handling::type_classification_mismatch<T>{};
                }
            }

            template<class T>
            using properties [[maybe_unused]] =
                prop::prop<queue_partition_point_type, detail2::queue_partition_point_t<T>>;
        };


        template<class edge_repr_t>
        struct node_indexed_or_mapped_range : any_of<
            node_indexed_range<adj_range<node_index, edge_repr_t>>,
            node_map_range<adj_range<node_key, edge_repr_t>>,
            indexed_range<keys::node, with_node_property<
                adj_range_if<node_index, edge_repr_t, keys::in_edge_list_container>,
                adj_range_if<node_index, edge_repr_t>
            >>,
            indexed_range<keys::node, with_node_property<
                partition_point,
                partitioned_queue<node_index, edge_repr_t>
            >>,
            map_range<keys::node, with_node_property<
                adj_range_if<node_key, edge_repr_t, keys::in_edge_list_container>,
                adj_range_if<node_key, edge_repr_t>
            >>,
            map_range<keys::node, with_node_property<
                partition_point,
                partitioned_queue<node_key, edge_repr_t>
            >>
        > {};

        struct adjacency_list : edge_index_as<node_indexed_or_mapped_range> {
            template<class T, class Props = get_properties<adjacency_list, T>>
            struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {

            };
        };

        template<class edge_t,
            class container_key = keys::out_edge_list_container,
            class container_size = std::conditional_t<
                std::is_same_v<container_key, keys::out_edge_list_container>,
                keys::node_container_size,
                keys::container_size<container_key>
            >, class ... Others>
        struct adj_matrix_opt :
            indexed_range<keys::node,
                any_of<
                    optional<edge_t>,
                    optional<range_impl<edge_t, keys::edge_list_container>>,
                    Others...
                >,
                container_key,
                container_size
            >
            {};

        template<class plus_type,
                 class container_key = keys::out_edge_list_container,
                 class container_size = std::conditional_t<
                     std::is_same_v<container_key, keys::out_edge_list_container>,
                     keys::node_container_size,
                     keys::container_size<container_key>
                 >>
        struct adj_matrix_inside :
            any_of<
                adj_matrix_opt<edge_property, container_key, container_size, bool_t, plus_type>,
                save_type<container_key, bitset<container_size>>
            > {};

        template<class edge_like_pro>
        struct node_indexed_or_adj_matr_edge :
            any_of<
                node_indexed_range<adj_matrix_opt<edge_like_pro>>,
                adj_matrix_opt<edge_like_pro, keys::edge_container>
            > {};

        template<>
        struct node_indexed_or_adj_matr_edge<edge_property> :
            any_of<
                node_indexed_range<adj_matrix_inside<range_impl<edge_property, keys::edge_list_container>>>,
                adj_matrix_inside<edge_count, keys::edge_container>
            > {};

        struct adjacency_matrix : edge_index_as<node_indexed_or_adj_matr_edge> {
            template<class T, class Props = get_properties<adjacency_matrix, T>>
            struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {

            };
        };

        struct edge_list  {
            template<class T, class Props = get_properties<edge_list, T>>
            struct [[maybe_unused]] graph_traits : graph_traits_common<Props> {

            };
        };
/*
        struct adjacency_matrix : any_of<
            optional_graph_property<
                any_of<
                    node_index_range_optional_node_property<
                        any_of<
                            node_index_range<any_of<
                                optional<index<edge>>,
                                optional<range<index<edge>>>,
                                range<index<edge>>
                            >>
                        >
                    >,
                    node_index_range<any_of<
                        optional<index<edge>>,
                        optional<range<index<edge>>>
                    >>
                >,
                edge_index_range<
                    edge_property
                >
            >,
            optional_graph_property<
                any_of<
                    node_index_range_optional_node_property<
                        any_of<
                            node_index_range<any_of<
                                optional<user_index<edge>>,
                                optional<range<user_index<edge>>>,
                                range<user_index<edge>>
                            >>
                        >
                    >,
                    node_index_range<any_of<
                        optional<user_index<edge>>,
                        optional<range<user_index<edge>>>
                    >>
                >,
                user_edge_index_map<
                    edge_property
                >
            >
        > {};

        struct edge_list : any_of<
            with_graph_property<edge_range_prop<node_index>>,
            with_graph_property<node_map_save<true, node_property>, edge_range_prop<node_index>>,
            with_graph_property<node_indexed_range<save_type<node_repr_type, node_property>>, edge_range_prop<node_index<index_t>>>
        > {};
        */

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


        template<class graph_property, class = void>
        constexpr static inline bool is_graph_property = false;

        template<class graph_property>
        constexpr static inline bool is_graph_property<graph_property, std::enable_if_t<
            detail2::is_constexpr<&detail2::constexpr_convertible<std::remove_reference_t<graph_property>,
                typename std::remove_reference_t<graph_property>::value_type>>(0) &&
                std::remove_reference_t<graph_property>::value_type::unknown ==
                std::remove_reference_t<graph_property>::value_type::unknown
        >> = (std::is_same_v<typename std::remove_reference_t<graph_property>::value_type, edge_direction_t> ||
            std::is_same_v<typename std::remove_reference_t<graph_property>::value_type, allow_parallel_edges_t>) &&
            std::remove_reference_t<graph_property>::value_type::unknown != std::remove_reference_t<graph_property>{};
    }
}

#endif //GRAPH_GRAPH_TRAITS_HPP
