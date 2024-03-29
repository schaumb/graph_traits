//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_RECOGNIZE_HPP
#define BXLX_GRAPH_RECOGNIZE_HPP

#include "../classify/classify.hpp"
#include "assert_types.hpp"
#include "properties.hpp"
#include <algorithm>
#include <functional>


namespace bxlx::graph::traits {
namespace type_filter {
  template <class Check = std::greater<>, std::size_t Size = 0, class Reason = empty_tuple>
  struct tuple_t {
    struct is_tuple {
      template<class T>
      using type = std::conditional_t<Check{}(std::tuple_size_v<T>, Size),
                                      std::true_type,
                                      assert_types::reason<Reason, assert_types::got<std::tuple_size_v<T>>>>;
    };
    struct is_not_tuple {
      template<class T>
      using type = assert_types::reason<expected_tuple, assert_types::got<classification::classify<T>>>;
    };

    template <class T>
    using is_valid = typename std::conditional_t<
          classification::classify<T> == classification::type::tuple_like,
          is_tuple,
          is_not_tuple>::template type<T>;
  };

  using tuple      = tuple_t<>;
  using tuple_gt_2 = tuple_t<std::greater<>, 2, tuple_size_not_greater_than_2>;
  using tuple_eq_2 = tuple_t<std::equal_to<>, 2, tuple_size_not_equal_to_2>;

  struct any {
    template <class T>
    using is_valid = std::true_type;
  };

  template <classification::type t>
  struct fix_typed_t {

    template<class T>
    using is_valid = std::conditional_t<
          classification::classify<T> == t,
          std::true_type,
          assert_types::reason<assert_types::expected<t, assert_types::got<classification::classify<T>>>>>;
  };

  template<class type_filter, class T>
  constexpr static bool is_valid_v = type_filter::template is_valid<T>::value;

  template<class type_filter, class T>
  using invalid_type_t = typename type_filter::template is_valid<T>;

  using range    = fix_typed_t<classification::type::range>;
  using map      = fix_typed_t<classification::type::map_like>;
  using index    = fix_typed_t<classification::type::index>;
  using bitset   = fix_typed_t<classification::type::bitset>;
  using optional = fix_typed_t<classification::type::optional>;
  using bool_v   = fix_typed_t<classification::type::bool_t>;
} // namespace type_filter

namespace next_type {
  template <class T, class U = std::make_index_sequence<std::tuple_size_v<T> - 1>, class = void>
  struct split_tuple_last;

  template <template <class...> class T, class... Types, std::size_t... Ix>
  struct split_tuple_last<T<Types...>, std::index_sequence<Ix...>, std::enable_if_t<(sizeof...(Ix) > 1)>> {
    using First [[maybe_unused]]    = std::tuple_element_t<0, T<Types...>>;
    using DropLast [[maybe_unused]] = T<std::tuple_element_t<Ix, T<Types...>>...>;
    using Last [[maybe_unused]]     = std::tuple_element_t<sizeof...(Ix), T<Types...>>;
  };

  template <template <class...> class T, class... Types, std::size_t... Ix>
  struct split_tuple_last<T<Types...>, std::index_sequence<Ix...>, std::enable_if_t<sizeof...(Ix) == 1>> {
    using First [[maybe_unused]]    = std::tuple_element_t<0, T<Types...>>;
    using Last [[maybe_unused]]     = std::tuple_element_t<sizeof...(Ix), T<Types...>>;
  };

  template <template <class...> class T, class... Types, std::size_t... Ix>
  struct split_tuple_last<const T<Types...>, std::index_sequence<Ix...>, std::enable_if_t<(sizeof...(Ix) > 1)>> {
    using First [[maybe_unused]]    = const std::tuple_element_t<0, T<Types...>>;
    using DropLast [[maybe_unused]] = const T<std::tuple_element_t<Ix, T<Types...>>...>;
    using Last [[maybe_unused]]     = const std::tuple_element_t<sizeof...(Ix), T<Types...>>;
  };

  template <template <class...> class T, class... Types, std::size_t... Ix>
  struct split_tuple_last<const T<Types...>, std::index_sequence<Ix...>, std::enable_if_t<sizeof...(Ix) == 1>> {
    using First [[maybe_unused]]    = const std::tuple_element_t<0, T<Types...>>;
    using Last [[maybe_unused]]     = const std::tuple_element_t<sizeof...(Ix), T<Types...>>;
  };

  template <class T>
  using tup_0 = typename split_tuple_last<T>::First;

  template <class T>
  using tup_N_1 = typename split_tuple_last<T>::Last;

  template <class T>
  using tup_to_N_1 = typename split_tuple_last<T>::DropLast;

  template <class T>
  using E = type_traits::range_value_t<T>;

  template <class T>
  using K = type_traits::map_key_t<T>;

  template <class T>
  using V = std::remove_reference_t<type_traits::map_value_ref_t<T>>;

  template <class T>
  using O = type_traits::optional_value_t<T>;

  template <class U>
  using T = U;

  template <class U>
  using CVT = std::remove_cv_t<U>;

  template <class U>
  using Dummy = std::enable_if_t<false, U>;
} // namespace next_type

template <class T>
struct crtp_address_to_nullptr {
  constexpr T const* operator&() const { return {}; }
};

namespace conditions {
  struct condition_t {};
  template <template <class> class Tr>
  struct not_range_t
        : crtp_address_to_nullptr<not_range_t<Tr>>
        , condition_t {
    template <class T, class>
    constexpr static std::conditional_t<
          !type_traits::is_range_v<Tr<T>>,
          std::true_type,
          assert_types::reason<assert_types::got<classification::type::range>, not_expected>>
    valid() {
      return {};
    }
  };
  template <template <class> class Tr = next_type::T>
  constexpr not_range_t<Tr> not_range{};

  struct random_access_t
        : crtp_address_to_nullptr<random_access_t>
        , condition_t {
    template <class T, class = properties::empty_t>
    constexpr static std::conditional_t<
          std::is_base_of_v<std::random_access_iterator_tag, type_traits::range_iterator_tag_t<T>>,
          std::true_type,
          assert_types::reason<assert_types::input<type_traits::range_iterator_tag_t<T>>, not_random_access_range>>
    valid() {
      return {};
    }
  } constexpr random_access{};

  struct not_multimap_t
        : crtp_address_to_nullptr<not_multimap_t>
        , condition_t {
    template <class T, class = properties::empty_t>
    constexpr static std::conditional_t<
          !type_traits::is_associative_multi_v<T>,
          std::true_type,
          assert_types::reason<expected<false, struct is_multi>>>
    valid() {
      return {};
    }
  } constexpr not_multimap{};


  struct separator_of_t
        : crtp_address_to_nullptr<separator_of_t>
        , condition_t {
    template <class T, class = properties::empty_t>
    constexpr static auto valid() {
      if constexpr (type_traits::is_range_v<std::tuple_element_t<1, T>>) {
        if constexpr (std::is_base_of_v<std::random_access_iterator_tag,
                                        type_traits::range_iterator_tag_t<std::tuple_element_t<1, T>>>) {
          if constexpr (!std::is_const_v<T> && !std::is_const_v<std::tuple_element_t<1, T>> && (!type_traits::detail::class_member_traits::has_push_front_v<std::tuple_element_t<1, T>,
                type_traits::range_value_t<std::tuple_element_t<1, T>> &&> && !std::is_array_v<std::tuple_element_t<1, T>>)) {
            return assert_types::reason<struct no_push_front>{};
          } else if constexpr (classification::classify<std::tuple_element_t<0, T>> == classification::type::index) {
            return std::true_type{};
          } else {
            return assert_types::reason<
                  expected<classification::type::index, got<classification::classify<std::tuple_element_t<0, T>>>>>{};
          }
        } else if constexpr (
              std::is_base_of_v<std::bidirectional_iterator_tag,
                                type_traits::range_iterator_tag_t<std::tuple_element_t<1, T>>> && (
              std::is_same_v<std::tuple_element_t<0, T>,
                             type_traits::detail::std_begin_t<std::add_const_t<std::tuple_element_t<1, T>>>> ||
              std::is_same_v<std::tuple_element_t<0, T>,
                             type_traits::detail::std_begin_t<std::tuple_element_t<1, T>>>)) {
          return std::true_type{};
        } else {
          return assert_types::reason<struct iterator>{};
        }
      } else {
        return assert_types::reason<separator_of_t, struct not_range>{};
      }
    }
  } constexpr separator_of{};

  struct no_t
        : crtp_address_to_nullptr<no_t>
        , condition_t {
    template <class, class>
    constexpr static std::true_type valid() {
      return {};
    }
  } constexpr* no{};


  template <class... Ts>
  struct all {
    template <class T, class Props>
    constexpr static auto valid() {
      constexpr bool all_valid = ((Ts::template valid<T, Props>().value) && ...);
      if constexpr (all_valid) {
        return std::true_type{};
      } else {
        return std::tuple_element_t<
              0, decltype(std::tuple_cat(
                       std::declval<std::conditional_t<Ts::template valid<T, Props>(), std::tuple<>,
                                                       std::tuple<Ts>>>()...))>::template valid<T, Props>();
      }
    }
  };

  template <class K, class V>
  struct is_t
        : crtp_address_to_nullptr<is_t<K, V>>
        , condition_t {
    template <class, class Props>
    constexpr static auto valid() {
      if constexpr (properties::is_valid_v<Props, K, V>) {
        return std::true_type{};
      } else {
        return assert_types::reason<struct wrong_property_for_key, K>{};
      }
    }
  };

  template <class K>
  struct exclude_t
        : crtp_address_to_nullptr<exclude_t<K>>
        , condition_t {
    template <class, class Props>
    constexpr static auto valid() {
      if constexpr (properties::has_property_v<Props, K>) {
        return assert_types::reason<already_contains_property, K>{};
      } else {
        return std::true_type{};
      }
    }
  };

  template <class K>
  constexpr exclude_t<K> exclude{};

  template <class T, class U>
  constexpr auto operator&&(const T&, const U&)
        -> std::enable_if_t<std::is_base_of_v<condition_t, T> && std::is_base_of_v<condition_t, U>, all<T, U>*> {
    return {};
  }

  template <class... Ts, class U>
  constexpr auto
  operator&&(const all<Ts...>*, const U&) -> std::enable_if_t<std::is_base_of_v<condition_t, U>, all<Ts..., U>*> {
    return {};
  }
} // namespace conditions


namespace props {
  struct property_t {};

  struct empty_t
        : crtp_address_to_nullptr<empty_t>
        , property_t {
    template <class>
    using type = properties::empty_t;
  } constexpr* empty{};

  template <class K, template <class> class Tr>
  struct set_t
        : crtp_address_to_nullptr<set_t<K, Tr>>
        , property_t
        , conditions::condition_t {
    template <class V>
    using type = properties::property_t<K, Tr<V>>;

    template <class T, class Props>
    constexpr static auto valid() {
      if constexpr (properties::is_valid_v<Props, K, Tr<T>>) {
        return std::true_type{};
      } else {
        return assert_types::reason<struct cannot_set_key, K>{};
      }
    }
  };

  template <class K, class V>
  struct fix_t
        : crtp_address_to_nullptr<fix_t<K, V>>
        , property_t {
    template <class>
    using type = properties::property_t<K, V>;
  };

  template <class... Ts>
  struct multi {
    template <class T>
    using type = properties::merge_properties_t<typename Ts::template type<T>...>;
  };

  template <class T, class U>
  constexpr auto operator+(const T&, const U&)
        -> std::enable_if_t<std::is_base_of_v<property_t, T> && std::is_base_of_v<property_t, U>, multi<T, U>*> {
    return {};
  }

  template <class... Ts, class U>
  constexpr auto
  operator+(const multi<Ts...>*, const U&) -> std::enable_if_t<std::is_base_of_v<property_t, U>, multi<Ts..., U>*> {
    return {};
  }

  template <class PreProp, auto* V, class T>
  using add_properties = properties::merge_properties_t<PreProp, typename std::remove_reference_t<decltype(*V)>::template type<T>>;
} // namespace props

namespace collect_values {
  struct na_t {
  } constexpr na{};
  constexpr std::true_type  t{};
  constexpr std::false_type f{};
} // namespace collect_values

namespace collect_props {
  template <class T, class... Acceptable>
  struct settable_prop {

    template <class U>
    constexpr auto
    operator==(U const&) const -> std::enable_if_t<(std::is_same_v<U, Acceptable> || ...), conditions::is_t<T, U>> {
      return {};
    }

    template <class U>
    constexpr auto
    operator=(U const&) const -> std::enable_if_t<(std::is_same_v<U, Acceptable> || ...), props::fix_t<T, U>> {
      return {};
    }
  };

  template <template <class> class Tr = next_type::T>
  constexpr props::set_t<struct graph_prop_t, Tr> graph_prop{};
  template <template <class> class Tr = next_type::T>
  constexpr props::set_t<struct node_prop_t, Tr> node_prop{};
  template <template <class> class Tr = next_type::T>
  constexpr props::set_t<struct edge_prop_t, Tr> edge_prop{};
  template <template <class> class Tr = next_type::CVT>
  constexpr props::set_t<struct edge_type_t, Tr> edge_type{};
  template <template <class> class Tr = next_type::CVT>
  constexpr props::set_t<struct node_type_t, Tr> node_type{};

  constexpr settable_prop<struct user_edge_t, collect_values::na_t, std::true_type, std::false_type> user_edge{};
  constexpr settable_prop<struct user_node_t, std::true_type, std::false_type>                       user_node{};
  constexpr settable_prop<struct in_edges_t, std::true_type, std::false_type>   in_edges{};
  constexpr settable_prop<struct multi_edge_t, std::true_type, std::false_type> multi_edge{};
  constexpr settable_prop<struct compressed_t, std::true_type, std::false_type> compressed{};
} // namespace collect_props

namespace state_machine {

  template<class T, std::size_t Index>
  using indexed_type = std::pair<T, std::integral_constant<std::size_t, Index>>;

  template<class CRTP>
  struct crtp_transform {
    using type = CRTP;
  };
  template<template<template <class> class> class Base, template <class> class Tr>
  struct crtp_transform<Base<Tr>> {
    using type = Base<next_type::Dummy>;
  };

  template<class T>
  using crtp_transform_t = typename crtp_transform<T>::type;

  template <class CRTPBase, class... States>
  struct any_of {
    template <class T, class PropsT = properties::empty_t>
    constexpr static auto valid() {
      using CRTP = crtp_transform_t<CRTPBase>;
      static_assert(!properties::has_property_v<PropsT, indexed_type<CRTP, 2>>);
      using Props =
            properties::merge_properties_t<PropsT, std::conditional_t<properties::has_property_v<PropsT, CRTP>,
                                                                      std::conditional_t<properties::has_property_v<PropsT, indexed_type<CRTP, 1>>,
                                                                                         properties::property_t<indexed_type<CRTP, 2>, T>,
                                                                                         properties::property_t<indexed_type<CRTP, 1>, T>
                                                                                         >,
                                                                      properties::property_t<CRTP, T>
                                                                      >>;
      constexpr std::size_t valid_states = ((States::template valid<T, Props>().value) + ...);
      constexpr std::size_t valid_types = ((States::template is_valid_type_v<T>) + ...);
      constexpr std::size_t valid_conditions = ((States::template valid_conditions<T, Props>()) + ...);
      if constexpr (valid_states == 1) {
        return std::tuple_element_t<
              0, decltype(std::tuple_cat(
                       std::declval<std::conditional_t<States::template valid<T, Props>(), std::tuple<States>,
                                                       std::tuple<>>>()...))>::template valid<T, Props>();
      } else if constexpr (valid_states > 1) {
        return assert_types::reason<
              multiple_good_recognition, assert_types::at<CRTP>, assert_types::input<typename assert_types::type_holder<T, PropsT>::type>,
              decltype(std::tuple_cat(
                    std::declval<std::conditional_t<States::template valid<T, Props>(),
                                                    std::tuple<decltype(States::template valid<T, Props>())>,
                                                    std::tuple<>>>()...))>{};
      } else if constexpr (valid_types == 1) {
        return std::tuple_element_t<
              0, decltype(std::tuple_cat(
                       std::declval<std::conditional_t<States::template is_valid_type_v<T>, std::tuple<States>,
                                                       std::tuple<>>>()...))>::template valid<T, Props>();
      } else if constexpr (valid_types > 1 &&
                           valid_conditions == 1) {
        return std::tuple_element_t<
              0, decltype(std::tuple_cat(std::declval<std::conditional_t<States::template valid_conditions<T, Props>(),
                                                                         std::tuple<States>, std::tuple<>>>()...))>::
              template valid<T, Props>();
      } else if constexpr (valid_conditions > 1) {
        return assert_types::reason<
              no_good_recognition, assert_types::at<CRTP>, assert_types::input<typename assert_types::type_holder<T, PropsT>::type>,
              assert_types::got<false,
                                decltype(std::tuple_cat(std::declval<std::conditional_t<
                                                              States::template valid_conditions<T, Props>(),
                                                              std::tuple<decltype(States::template valid<T, Props>())>,
                                                              std::tuple<>>>()...))>>{};
      } else if constexpr (valid_types > 1) {
        return assert_types::reason<
              no_good_recognition, assert_types::at<CRTP>, assert_types::input<typename assert_types::type_holder<T, PropsT>::type>,
              assert_types::got<false,
                                decltype(std::tuple_cat(std::declval<std::conditional_t<
                                                              States::template is_valid_type_v<T>,
                                                              std::tuple<decltype(States::template valid<T, Props>())>,
                                                              std::tuple<>>>()...))>>{};
      } else {
        return assert_types::reason<no_good_recognition, assert_types::at<CRTP>, assert_types::input<typename assert_types::type_holder<T, PropsT>::type>>{};
      }
    }
  };

  template <template <class> class Tr, class Base>
  struct transform : Base {
    template <class T, class Props>
    constexpr static auto valid() {
      return Base::template valid<Tr<T>, Props>();
    }
  };


  template <class CRTP, template <class> class Tr, class... NextStates>
  using any_of_r = transform<Tr, any_of<CRTP, NextStates...>>;

  using namespace type_filter;
  using namespace conditions;
  using namespace props;
  using namespace next_type;
  using namespace collect_props;
  using namespace collect_values;

  template <class Properties>
  struct valid_props : std::true_type {
    using properties [[maybe_unused]] = Properties;
  };

  template<class T, class Props, class... NextStates>
  struct apply_properties {
    using type = valid_props<Props>;
  };

  template<class T, class Props, class... NextStates>
  using apply_properties_t = typename apply_properties<T, Props, NextStates...>::type;

  template<class, class T, class Props, class State, class...NextStates>
  struct apply_properties_impl;

  template<class T, class Props, class State, class...NextStates>
  struct apply_properties_impl<std::true_type, T, Props, State, NextStates...> {
    using type = apply_properties_t<T, typename decltype(State::template valid<T, Props>())::properties,
                                    NextStates...>;
  };
  template<class T, class Props, class State, class...NextStates>
  struct apply_properties_impl<std::false_type, T, Props, State, NextStates...> {
    using type = decltype(State::template valid<T, Props>());
  };

  template<class T, class Props, class State, class...NextStates>
  struct apply_properties<T, Props, State, NextStates...> :
        apply_properties_impl<std::bool_constant<(State::template valid<T, Props>())>, T, Props, State, NextStates...> {};


  template <class TypeFilter = any, auto* Conditions = no, auto* Properties = empty, class... NextStates>
  struct transition {
    template <class T, class Props = properties::empty_t>
    constexpr static auto valid() {
      if constexpr (!is_valid_type_v<T>) {
        return invalid_type_t<TypeFilter, T>{};
      } else if constexpr (valid_conditions<T, Props>()) {
        return apply_properties_t<T, add_properties<Props, Properties, T>, NextStates...>{};
      } else {
        return std::remove_reference_t<decltype(*Conditions)>::template valid<T, Props>();
      }
    }

    template <class T>
    constexpr static bool is_valid_type_v = is_valid_v<TypeFilter, T>;

    template <class T, class Props = properties::empty_t>
    constexpr static bool valid_conditions() {
      if constexpr (is_valid_type_v<T>) {
        return std::remove_reference_t<decltype(*Conditions)>::template valid<T, Props>().value;
      } else {
        return false;
      }
    }
  };

  template <template <template <class> class...> class State>
  using simple_transition = transition<any, no, empty, State<>>;


  template <template <class> class Tr>
  struct edge_container
        : any_of_r<edge_container<Tr>,
                   Tr,
                   transition<range, random_access && not_range<E>, edge_prop<E> + (user_edge = f)>,
                   transition<map, &not_range<V>, edge_type<K> + edge_prop<V> + (user_edge = t)>> {};

  template <template <class> class Tr>
  struct node_container
        : any_of_r<node_container<Tr>,
                   Tr,
                   transition<range, random_access && not_range<E>, node_prop<E> + (user_node = f)>,
                   transition<map, not_multimap && not_range<V>, node_type<K> + node_prop<V> + (user_node = t)>> {};


  template <template <class> class Tr = T>
  struct node
        : any_of_r<node<Tr>,
                   Tr,
                   transition<index, user_node == f && node_type<>, &node_type<>>,
                   transition<any, user_node == t && node_type<>, &node_type<>>> {};

  template <template <class> class Tr = T>
  struct edge
        : any_of_r<edge<Tr>,
                   Tr,
                   transition<index, user_edge == f && edge_type<>, &edge_type<>>,
                   transition<any, user_edge == t && edge_type<>>,
                   transition<any, user_edge == na && exclude<edge_prop_t> && not_range<>, &edge_prop<>>> {};

  template <template <class> class Tr = T>
  struct adj_list_cont_elem
        : any_of_r<adj_list_cont_elem<Tr>,
                   Tr,
                   transition<tuple_eq_2, no, empty, node<tup_0>, edge<tup_N_1>>,
                   simple_transition<node>> {};

  template <template <class> class Tr = T>
  struct adj_list_cont
        : any_of_r<adj_list_cont<Tr>,
                   Tr,
                   transition<range, no, empty, adj_list_cont_elem<E>>,
                   transition<map, no, empty, node<K>, edge<V>>> {};

  template <template <class> class Tr = T>
  struct adj_list_node_p
        : any_of_r<adj_list_node_p<Tr>,
                   Tr,
                   transition<tuple_eq_2, &separator_of, &(in_edges = t), adj_list_cont<tup_N_1>>,
                   transition<tuple_eq_2, no, &(in_edges = t), adj_list_cont<tup_0>, adj_list_cont<tup_N_1>>,
                   transition<any, no, &(in_edges = f), adj_list_cont<>>> {};

  template <template <class> class Tr = T>
  struct adj_list_node
        : any_of_r<adj_list_node<Tr>,
                   Tr,
                   transition<tuple_gt_2, &not_range<tup_N_1>, &node_prop<tup_N_1>, adj_list_node_p<tup_to_N_1>>,
                   transition<tuple_eq_2, &not_range<tup_N_1>, &node_prop<tup_N_1>, adj_list_node_p<tup_0>>,
                   simple_transition<adj_list_node_p>> {};

  struct adj_list
        : any_of<adj_list,
                 transition<range, &random_access, &(user_node = f), adj_list_node<E>>,
                 transition<map, &not_multimap, (user_node = t) + node_type<K>, adj_list_node<V>>> {};

  struct adj_mat_elem_opt
        : any_of_r<adj_mat_elem_opt,
                   O,
                   transition<range, &(multi_edge == t), &(multi_edge = t), edge<E>>,
                   transition<any, &(multi_edge == f), &(multi_edge = f), edge<>>> {};

  template <template <class> class Tr>
  struct adj_mat_elem
        : any_of_r<adj_mat_elem<Tr>,
                   Tr,
                   transition<optional, no, empty, adj_mat_elem_opt>,
                   transition<range, compressed == f && multi_edge == t, &(multi_edge = t), edge<E>>,
                   transition<bool_v, user_edge == na && multi_edge == f, &(multi_edge = f)>> {};

  template <template <class> class Tr = T>
  struct adj_mat_cont
        : any_of_r<adj_mat_cont<Tr>,
                   Tr,
                   transition<bitset, user_edge == na && multi_edge == f, &(multi_edge = f)>,
                   transition<range, &random_access, empty, adj_mat_elem<E>>> {};

  template <template <class> class Tr = T>
  struct adj_mat_node_p
        : any_of_r<adj_mat_node_p<Tr>,
                   Tr,
                   transition<tuple_eq_2, no, &(in_edges = t), adj_mat_cont<tup_0>, adj_mat_cont<tup_N_1>>,
                   simple_transition<adj_mat_cont>> {};

  template <template <class> class Tr = T>
  struct adj_mat_node
        : any_of_r<adj_mat_node<Tr>,
                   Tr,
                   transition<tuple_gt_2, &not_range<tup_N_1>, &node_prop<tup_N_1>, adj_mat_node_p<tup_to_N_1>>,
                   transition<tuple_eq_2, &not_range<tup_N_1>, &node_prop<tup_N_1>, adj_mat_node_p<tup_0>>,
                   simple_transition<adj_mat_node_p>> {};

  struct adj_mat
        : any_of<adj_mat,
                 transition<range, &random_access, &(compressed = f), adj_mat_node<E>>,
                 transition<any, no, &(compressed = t), adj_mat_cont<>>> {};


  template <template <class> class Tr = T>
  struct node_pair : any_of_r<node_pair<Tr>,
                              Tr,
                              transition<tuple_eq_2, no, empty, node<tup_0>, node<tup_N_1>>> {};

  template <template <class> class Tr>
  struct edge_list_elem
        : any_of_r<edge_list_elem<Tr>,
                   Tr,
                   transition<tuple_gt_2, no, empty, edge<tup_N_1>, node_pair<tup_to_N_1>>,
                   transition<tuple_eq_2, no, empty, edge<tup_N_1>, node_pair<tup_0>>,
                   simple_transition<node_pair>> {};

  template <template <class> class Tr = T>
  struct edge_list_n
        : any_of_r<edge_list_n<Tr>,
                   Tr,
                   transition<range, no, empty, edge_list_elem<E>>,
                   transition<map, no, empty, edge<V>, node_pair<K>>> {};

  struct edge_list
        : any_of<edge_list,
                 transition<tuple_eq_2, no, empty, node_container<tup_0>, edge_list_n<tup_N_1>>,
                 transition<any, no, &(user_node = t), edge_list_n<>>> {};

  template <template <class> class Tr = T>
  struct graph_ep
        : any_of_r<graph_ep<Tr>,
                   Tr,
                   transition<any, no, empty, adj_list>,
                   transition<any, no, empty, adj_mat>,
                   transition<any, no, empty, edge_list>> {};

  template <template <class> class Tr = T>
  struct graph_p
        : any_of_r<graph_p<Tr>,
                   Tr,
                   transition<tuple_gt_2, no, empty, edge_container<tup_N_1>, graph_ep<tup_to_N_1>>,
                   transition<tuple_eq_2, no, empty, edge_container<tup_N_1>, graph_ep<tup_0>>,
                   transition<any, no, &(user_edge = na), graph_ep<>>> {};

  struct graph
        : any_of<graph,
                 transition<tuple_gt_2, &not_range<tup_N_1>, &graph_prop<tup_N_1>, graph_p<tup_to_N_1>>,
                 transition<tuple_eq_2, &not_range<tup_N_1>, &graph_prop<tup_N_1>, graph_p<tup_0>>,
                 simple_transition<graph_p>> {};
} // namespace state_machine
} // namespace bxlx::graph::traits

#endif //BXLX_GRAPH_RECOGNIZE_HPP
