//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef GRAPH_GRAPH_TRAITS_HPP
#define GRAPH_GRAPH_TRAITS_HPP

#include "type_classification.hpp"

namespace bxlx {
    namespace traits {
        enum class graph_representation {
            adjacency_list,
            adjacency_matrix,
            adjacency_bitset,
            edge_list,
        };

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
        using identity = std::common_type<T>;

        template<class, std::size_t = 0, class = void>
        struct get_inside_type {};

        template<class T, std::size_t iTh>
        struct get_inside_type<T, iTh, std::enable_if_t<tuple_with_size<iTh>::template type<T>::value>>
            : identity<std::tuple_element_t<iTh, T>> {};

        template<class T>
        struct get_inside_type<T, 0, std::enable_if_t<detail2::classify<T> == detail2::type_classification::compile_time_random_access_range>>
            : identity<std::tuple_element_t<0, T>> {};

        template<class T>
        struct get_inside_type<T, 0, std::enable_if_t<detail2::is_range_v<T> && detail2::classify<T> != detail2::type_classification::compile_time_random_access_range>>
            : identity<typename detail2::range_traits<T>::value_type> {};

        template<class T>
        struct get_inside_type<T, 0, std::enable_if_t<detail2::is_optional_v<T>>>
            : identity<typename detail2::optional_traits<T>::value_type> {};

        template<class T>
        struct get_inside_type<T, 0, std::enable_if_t<detail2::is_bitset_like_v<T>>>
            : identity<bool> {};

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


        template<class Condition, class ToState, template<class> class Transform = identity, class ... Additional>
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
/*
            template<class SubType, class Graph, class Getter>
            struct type<SubType, Graph, Getter, std::true_type> : type<SubType, Graph, Getter, void> {
                constexpr static auto in_storage_size = 0;
            };
*/
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
                state_transition<tuple_with_size<3>, end_state, identity, getter_t<2>>
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
    }
}

#endif //GRAPH_GRAPH_TRAITS_HPP
