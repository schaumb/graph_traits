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
    namespace traits {
        template<class T, class V = std::make_index_sequence<std::tuple_size_v<T>>>
        struct properties;
        template<class T, std::size_t ...ix>
        struct properties<T, std::index_sequence<ix...>> {
            using type = T;
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
            using merged_type = properties<decltype(std::tuple_cat(std::declval<T>(), std::declval<typename Others::type>()...))>;
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

        struct no_prop {
            template<class T>
            using properties = empty_properties;
        };

        struct edge_property : always_valid {
            template<class T>
            using properties = property<edge_property, T>;
        };
        struct node_property : always_valid {
            template<class T>
            using properties = property<node_property, T>;
        };
        struct graph_property : always_valid {
            template<class T>
            using properties = property<graph_property, T>;
        };

        struct node_index : always_valid {
            template<class T>
            using properties = merge_properties<property<struct user_node_index, std::true_type>, property<node_index, T>>;
        };

        struct index : accept_only<detail2::type_classification::index> {
            template<class T>
            using properties = merge_properties<property<struct user_node_index, std::false_type>, property<node_index, T>>;
        };

        struct bool_t : accept_only<detail2::type_classification::bool_t>, no_prop {};
        struct bitset : accept_only<detail2::type_classification::bitset_like_container> {
            template<class T>
            using properties = property<struct inside_container_size, constant_t<bxlx::detail2::compile_time_size_v<T>>>;
        };

        template<class Cond>
        struct optional : accept_recursively<optional<Cond>, detail2::type_classification::optional> {
            template<class T>
            constexpr static bool is_valid_nested() {
                return Cond::template is_valid<detail2::optional_traits_type<T>>();
            }

            template<class T>
            using properties = get_properties<Cond, detail2::optional_traits_type<T>>;
        };


        template<class... Conds>
        struct any_of {
            template<class T>
            constexpr static bool is_valid() {
                return (Conds::template is_valid<T>() + ...) == 1;
            }

            template<class T>
            constexpr static std::size_t the_valid_index() {
                std::size_t ix{};
                std::size_t res = ~std::size_t{};
                static_cast<void>((..., (Conds::template is_valid<T>() ? res = ix : ++ix)));
                return res;
            }

            template<class T>
            using properties = get_properties<std::tuple_element_t<the_valid_index<T>(), std::tuple<Conds...>>, T>;

            template<class T>
            using graph_traits = typename std::tuple_element_t<the_valid_index<T>(), std::tuple<Conds...>>::template graph_traits<T>;
        };

        template<class ...Conds>
        struct tuple_like : accept_recursively<tuple_like<Conds...>, detail2::type_classification::tuple_like> {
            template<class T, std::size_t...Ix>
            static merge_properties<empty_properties, get_properties<Conds, std::tuple_element_t<Ix, T>>...>
            merge_properties_helper(std::index_sequence<Ix...>) { return {}; }

            template<class T>
            using properties = decltype(merge_properties_helper<T>(std::make_index_sequence<std::tuple_size_v<T>>{}));

            template<class T, std::size_t ...Ix>
            constexpr static bool is_valid_nested(std::index_sequence<Ix...>) {
                return (Conds::template is_valid<std::tuple_element_t<Ix, T>>() && ...);
            }

            template<class T>
            constexpr static bool is_valid_nested() {
                if constexpr (sizeof...(Conds) == std::tuple_size_v<T>) {
                    if constexpr (is_valid_nested<T>(std::make_index_sequence<std::tuple_size_v<T>>{})) {
                        return properties<T>::is_valid;
                    }
                }
                return false;
            }
        };

        template<class Cond>
        struct range_impl {
            template<class T>
            constexpr static bool is_valid_nested() {
                if constexpr (Cond::template is_valid<detail2::range_traits_type<T>>()) {
                    return properties<T>::is_valid;
                }
                return false;
            }

            template<class T>
            using properties = merge_properties<get_properties<Cond, detail2::range_traits_type<T>>, property<
                std::conditional_t<has_property<get_properties<Cond, detail2::range_traits_type<T>>, struct inside_container_size>,
                    struct outside_container_size,
                    struct inside_container_size
                >,
                constant_t<bxlx::detail2::compile_time_size_v<T>>
            >>;
        };

        template<class T>
        struct range : accept_recursively<range_impl<T>,
            detail2::type_classification::range,
            detail2::type_classification::sized_range,
            detail2::type_classification::random_access_range>, range_impl<T> {};

        template<class T>
        struct sized_range : accept_recursively<range_impl<T>,
            detail2::type_classification::sized_range,
            detail2::type_classification::random_access_range>, range_impl<T> {};

        template<class T>
        struct random_access_range : accept_recursively<random_access_range<T>,
            detail2::type_classification::random_access_range> {

            template<class U>
            using properties = merge_properties<get_properties<range_impl<T>, U>, property<struct user_node_index, std::false_type>>;

            template<class U>
            constexpr static bool is_valid_nested() {
                if constexpr (range_impl<T>::template is_valid_nested<U>()) {
                    return properties<U>::is_valid;
                }
                return false;
            }
        };

        template<class T, class U>
        struct map : accept_recursively<range_impl<tuple_like<T, U>>,
            detail2::type_classification::map_like_container>, range_impl<tuple_like<T, U>> {};

        template<class T, class Property>
        using with_property = any_of<tuple_like<T, Property>, T>;

        template<class T>
        using with_graph_property = with_property<T, graph_property>;
        template<class T>
        using with_node_property = with_property<T, node_property>;
        template<class T>
        using with_edge_property = with_property<T, edge_property>;


        struct adjacency_list : with_graph_property<any_of<
            random_access_range<with_node_property<range<with_edge_property<index>>>>,
            map<node_index, with_node_property<any_of<
                range<with_edge_property<node_index>>,
                map<node_index, edge_property>
            >>>
        >> {
            template<class T>
            struct graph_traits {
                using using_properties = get_properties<adjacency_list, T>;
                constexpr static inline auto representation = graph_representation::adjacency_list;
            };
        };

        struct adjacency_matrix : with_graph_property<
            random_access_range<with_node_property<any_of<
                random_access_range<any_of<
                    bool_t,
                    optional<edge_property>
                >>,
                bitset
            >>>
        > {
            template<class T>
            struct graph_traits {
                using using_properties = get_properties<adjacency_matrix, T>;
                constexpr static inline auto representation = graph_representation::adjacency_matrix;
            };
        };

        struct edge_list : with_graph_property<sized_range<any_of<
            tuple_like<node_index, node_index>,
            tuple_like<node_index, node_index, edge_property>
        >>> {
            template<class T>
            struct graph_traits {
                using using_properties = get_properties<edge_list, T>;
                constexpr static inline auto representation = graph_representation::edge_list;
            };
        };

        struct graph : any_of<adjacency_list, adjacency_matrix, edge_list> {};

        template<class T>
        constexpr inline bool is_graph_v = graph::template is_valid<T>();

        template<class T>
        using graph_traits = typename graph::template graph_traits<T>;

        /*
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

        struct any {
            template<class>
            using type = std::true_type;
        };



        template<detail2::type_classification ... cl>
        struct classified_with {
            template<class T>
            using type = std::bool_constant<((detail2::classify<T> == cl) || ...)>;
        };

        template<std::size_t Size, class C = std::less_equal<>>
        struct tuple_with_size {
            template<class T, class = void>
            struct type_t : std::false_type {};

            template<class T>
            struct type_t<T, std::enable_if_t<detail2::classify<T> == detail2::type_classification::tuple_like>>
                : std::bool_constant<C{}(Size, std::tuple_size_v<T>)> {};

            template<class T>
            using type = type_t<T>;
        };


        template<class T>
        using type_identity = std::common_type<T>;

        template<class, std::size_t = 0, class = void>
        struct get_inside_type {};

        template<class T, std::size_t iTh>
        struct get_inside_type<T, iTh, std::enable_if_t<tuple_with_size<iTh>::template type<T>::value>>
            : type_identity<std::tuple_element_t<iTh, T>> {};

        template<class T>
        struct get_inside_type<T, 0, std::enable_if_t<detail2::classify<T> == detail2::type_classification::compile_time_random_access_range>>
            : type_identity<std::tuple_element_t<0, T>> {};

        template<class T>
        struct get_inside_type<T, 0, std::enable_if_t<detail2::is_range_v<T> && detail2::classify<T> != detail2::type_classification::compile_time_random_access_range>>
            : type_identity<typename detail2::range_traits<T>::value_type> {};

        template<class T>
        struct get_inside_type<T, 0, std::enable_if_t<detail2::is_optional_v<T>>>
            : type_identity<typename detail2::optional_traits<T>::value_type> {};

        template<class T>
        struct get_inside_type<T, 0, std::enable_if_t<detail2::is_bitset_like_v<T>>>
            : type_identity<bool> {};

        template<class T>
        using get_inside_type_t = get_inside_type<T>;

        template<std::size_t iTh>
        struct get_nth_inside_type {
            template<class T>
            using type = get_inside_type<T, iTh>;
        };



        struct constexpr_size_v {
            template<class T, class = void>
            constexpr static std::size_t value = std::size(T{});

            template<class T>
            constexpr static std::size_t value<T, std::enable_if_t<detail2::is_tuple_like_v<T>>> = std::tuple_size_v<T>;
        };
        template<auto T = std::size_t{}>
        struct constant {
            template<class>
            constexpr static auto value = T;
        };


        template<class Condition, class ToState, template<class> class Transform = type_identity, class ... Additional>
        struct state_transition {
            template<class T>
            static constexpr bool valid = Condition::template type<T>::value;

            using to_state = ToState;
            template<class T>
            using transform = typename Transform<T>::type;

            template<class Subgraph, class Graph, template <class, class, class...> class Type>
            using type = Type<Subgraph, Graph, Additional...>;
        };

        template<class ... Ts>
        struct transitions {
            template<class T>
            constexpr static bool valid = (Ts::template valid<T> || ...);

            template<class T, class Check, class ...Others>
            constexpr static auto get_transition() {
                if constexpr (Check::template valid<T>)
                    return Check{};
                else
                    return get_transition<T, Others...>();
            }

            template<class T>
            using to_transition = decltype(get_transition<T, Ts...>());
        };

        struct end_state;

        using type = detail2::type_classification;

        struct graph_property {
            using tr = transitions<
                state_transition<tuple_with_size<2>, struct graph_data, get_inside_type_t, getter_t<0>, getter_t<1>>,
                state_transition<classified_with<
                    type::compile_time_random_access_range,
                    type::random_access_range,
                    type::sized_range,
                    type::map_like_container>, struct graph_data>>;

            template<class, class Graph, class GetData = identity_t, class GetGraphImpl = noop_t>
            struct type {
                constexpr static auto get_data = GetData{};
                constexpr static auto get_graph_property = GetGraphImpl{};
                using graph_repr_type = Graph;
            };
        };

        struct graph_data {
            using tr = transitions<
                state_transition<classified_with<type::compile_time_random_access_range>, struct graph_rar, get_inside_type_t, constexpr_size_v>,
                state_transition<classified_with<type::random_access_range>, struct graph_rar, get_inside_type_t>,
                state_transition<classified_with<type::sized_range, type::map_like_container>, struct graph_array, get_inside_type_t,
                    constant<>, std::true_type>
            >;

            template<class SubType, class Graph, class Getter = constant<>, class =
                std::bool_constant<SubType::representation == graph_representation::edge_list>>
            struct type {
                constexpr static auto storage_size = Getter::template value<Graph>;
            };

            template<class SubType, class Graph, class Getter>
            struct type<SubType, Graph, Getter, std::false_type> : type<SubType, Graph, Getter, void> {
                using node_index_t = decltype(std::size(std::declval<Graph&>()));
            };
        };

        struct graph_rar {
            using tr = transitions<
                state_transition<classified_with<type::compile_time_random_access_range>, struct graph_rar_in, get_inside_type_t, constexpr_size_v>,
                state_transition<classified_with<type::compile_time_bitset_like_container>, struct graph_bitset, get_inside_type_t, constexpr_size_v>
            >;
            template<class SubType, class Graph, class Getter = constant<>>
            struct type {
                constexpr static auto in_storage_size = Getter::template value<Graph>;
            };
        };

        struct graph_array {
            using tr = transitions<
                state_transition<tuple_with_size<2>, end_state>,
                state_transition<tuple_with_size<3>, end_state, type_identity, getter_t<2>>
            >;

            template<class SubType, class Graph, class EdgeProp = noop_t>
            struct type {
                constexpr static graph_representation representation = graph_representation::edge_list;
                constexpr static auto get_edge_property = EdgeProp{};
                using edge_repr_type = Graph;

                constexpr static auto get_node_property = noop_t{};
                using node_repr_type = void*;

                using node_index_t = std::common_type_t<detail2::remove_cvref_t<std::tuple_element_t<0, Graph>>,
                                                        detail2::remove_cvref_t<std::tuple_element_t<1, Graph>>>;

                constexpr static auto edge_source = getter_t<0>{};
                constexpr static auto edge_target = getter_t<1>{};
            };
        };
*/
        /*
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
         */
/*
        template<class Graph, class State = graph_property, class = void>
        struct get_graph_traits_type {
            static_assert(State::tr::template valid<Graph>, "No valid state for this class");
        };

        template<class Graph, class State>
        struct get_graph_traits_type<Graph, State, std::enable_if_t<std::is_same_v<State, end_state>>> {
            using type = end_state;
        };

        template<class Graph, class State>
        struct get_graph_traits_type<Graph, State, std::enable_if_t<State::tr::template valid<Graph>>> {
            using Transition = typename State::tr::template to_transition<Graph>;
            using SubType = typename get_graph_traits_type<typename Transition::template transform<Graph>, typename Transition::to_state>::type;
            struct type : Transition::template type<SubType, Graph, State::template type>, SubType
            {};
        };

        template<class Graph>
        using graph_traits = typename get_graph_traits_type<Graph>::type;
        */
    }
}

#endif //GRAPH_GRAPH_TRAITS_HPP
