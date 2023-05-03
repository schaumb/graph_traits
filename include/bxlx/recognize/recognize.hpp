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
#include "graph_traits.hpp"
#include "properties.hpp"
#include <algorithm>
#include <functional>


namespace bxlx::graph::traits {
namespace type_filter {
  template <class Check = std::greater<>, std::size_t Size = 0, class Reason = empty_tuple>
  struct tuple_t {
    template <class T, class = std::enable_if_t<classification::classify<T> != classification::type::tuple_like>>
    constexpr static assert_types::reason<expected_tuple, T> valid() {
      return {};
    }

    template <class T, class = std::enable_if_t<classification::classify<T> == classification::type::tuple_like>>
    constexpr static auto valid() {
      if constexpr (Check{}(std::tuple_size_v<T>, Size)) {
        return std::true_type{};
      } else {
        return assert_types::reason<Reason, T>{};
      }
    }
  };

  using tuple      = tuple_t<>;
  using tuple_gt_2 = tuple_t<std::greater<>, 2, tuple_size_not_greater_than_2>;
  using tuple_eq_2 = tuple_t<std::equal_to<>, 2, tuple_size_not_equal_to_2>;

  struct any {
    template <class>
    constexpr static std::true_type valid() {
      return {};
    }
  };

  template <classification::type t>
  struct fix_typed_t {

    template <class T>
    constexpr static std::conditional_t<
          classification::classify<T> == t,
          std::true_type,
          assert_types::reason<assert_types::expected<t, assert_types::got<classification::classify<T>, T>>>>
    valid() {
      return {};
    }
  };

  using range    = fix_typed_t<classification::type::range>;
  using map      = fix_typed_t<classification::type::map_like>;
  using index    = fix_typed_t<classification::type::index>;
  using bitset   = fix_typed_t<classification::type::bitset>;
  using optional = fix_typed_t<classification::type::optional>;
  using bool_v   = fix_typed_t<classification::type::bool_t>;
} // namespace type_filter

namespace next_type {
  template <class T, class U = std::make_index_sequence<std::tuple_size_v<T> - 1>>
  struct split_tuple_last;

  template <template <class...> class T, class... Types, std::size_t... Ix>
  struct split_tuple_last<T<Types...>, std::index_sequence<Ix...>> {
    using First [[maybe_unused]]    = std::tuple_element_t<0, T<Types...>>;
    using DropLast [[maybe_unused]] = T<std::tuple_element_t<Ix, T<Types...>>...>;
    using Last [[maybe_unused]]     = std::tuple_element_t<sizeof...(Ix), T<Types...>>;
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
  using V = type_traits::map_key_t<T>;

  template <class T>
  using O = type_traits::optional_value_t<T>;

  template <class U>
  using T = U;
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
          classification::classify<Tr<T>> != classification::type::range,
          std::true_type,
          assert_types::reason<assert_types::got<classification::type::range, Tr<T>>, not_expected>>
    valid() {
      return {};
    }
  };
  template <template <class> class Tr = next_type::T>
  constexpr not_range_t<Tr> not_range{};

  struct random_access_t
        : crtp_address_to_nullptr<random_access_t>
        , condition_t {
    template <class T, class>
    constexpr static std::conditional_t<
          std::is_base_of_v<std::random_access_iterator_tag, type_traits::range_iterator_tag_t<T>>,
          std::true_type,
          assert_types::reason<assert_types::input<type_traits::range_iterator_tag_t<T>, T>, not_random_access_range>>
    valid() {
      return {};
    }
  } constexpr random_access{};

  struct not_multimap_t
        : crtp_address_to_nullptr<not_multimap_t>
        , condition_t {
    template <class T, class>
    constexpr static std::conditional_t<
          type_traits::is_associative_multi_v<T>,
          std::true_type,
          assert_types::reason<assert_types::input<T>, expected<true, struct is_associative>>>
    valid() {
      return {};
    }
  } constexpr not_multimap{};


  struct separator_of_t
        : crtp_address_to_nullptr<separator_of_t>
        , condition_t {
    template <class T, class>
    constexpr static auto valid() {
      if constexpr (type_traits::is_range_v<std::tuple_element_t<1, T>>) {
        if constexpr (std::is_base_of_v<std::random_access_iterator_tag,
                                        type_traits::range_iterator_tag_t<std::tuple_element_t<1, T>>>) {
          if constexpr (classification::classify<std::tuple_element_t<0, T>> == classification::type::index) {
            return std::true_type{};
          } else {
            return assert_types::reason<
                  expected<classification::type::index, got<classification::classify<std::tuple_element_t<0, T>>>>>{};
          }
        } else if constexpr (
              std::is_same_v<std::tuple_element_t<0, T>,
                             type_traits::detail::std_begin_t<std::add_const_t<std::tuple_element_t<1, T>>>>) {
          return std::true_type{};
        } else {
          return assert_types::reason<struct iterator, std::tuple_element_t<0, T>>{};
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
      if constexpr ((Ts::template valid<T, Props>() && ...)) {
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
      if constexpr (Props{}.is_valid(properties::property<K, V>{})) {
        return std::true_type{};
      } else {
        return assert_types::reason<assert_types::expected<&properties::property_v<K, V>>,
                                    assert_types::got<&properties::property_v<
                                          K, typename decltype(Props{}.template has_property<K>())::type>>>{};
      }
    }
  };

  template <class K>
  struct exclude_t
        : crtp_address_to_nullptr<exclude_t<K>>
        , condition_t {
    template <class, class Props>
    constexpr static auto valid() {
      if constexpr (Props{}.template has_property<K>()) {
        return assert_types::reason<already_contains_property, assert_types::expected<no, K>,
                                    assert_types::got<&properties::property_v<
                                          K, typename decltype(Props{}.template has_property<K>())::type>>>{};
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
    using type = properties::properties<>;
  } constexpr* empty{};

  template <class K, template <class> class Tr>
  struct set_t
        : crtp_address_to_nullptr<set_t<K, Tr>>
        , property_t
        , conditions::condition_t {
    template <class V>
    using type = properties::properties<properties::property<K, Tr<V>>>;

    template <class T, class Props>
    constexpr static auto valid() {
      if constexpr (Props{}.is_valid(properties::property<K, Tr<T>>{})) {
        return std::true_type{};
      } else {
        return assert_types::reason<assert_types::expected<&properties::property_v<K, Tr<T>>>,
                                    assert_types::got<&properties::property_v<
                                          K, typename decltype(Props{}.template has_property<K>())::type>>>{};
      }
    }
  };

  template <class K, class V>
  struct fix_t
        : crtp_address_to_nullptr<fix_t<K, V>>
        , property_t {
    template <class>
    using type = properties::properties<properties::property<K, V>>;
  };

  template <class... Ts>
  struct multi {
    template <class T>
    using type = decltype((std::declval<typename Ts::template type<T>>() + ...));
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
  using add_properties = decltype(PreProp{} + typename std::remove_reference_t<decltype(*V)>::template type<T>{});
} // namespace props

namespace collect_values {
  struct na_t {
  } constexpr na{};
  struct adj_list_t {
  } constexpr adj_list_v{};
  struct adj_mat_t {
  } constexpr adj_mat_v{};
  struct edge_list_t {
  } constexpr edge_list_v{};
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
  template <template <class> class Tr = next_type::T>
  constexpr props::set_t<struct edge_type_t, Tr> edge_type{};
  template <template <class> class Tr = next_type::T>
  constexpr props::set_t<struct node_type_t, Tr> node_type{};

  constexpr settable_prop<struct user_edge_t, collect_values::na_t, std::true_type, std::false_type> user_edge{};
  constexpr settable_prop<struct user_node_t, std::true_type, std::false_type>                       user_node{};
  constexpr settable_prop<struct type_t,
                          collect_values::adj_list_t,
                          collect_values::adj_mat_t,
                          collect_values::edge_list_t>
                                                                                type{};
  constexpr settable_prop<struct in_edges_t, std::true_type, std::false_type>   in_edges{};
  constexpr settable_prop<struct multi_edge_t, std::true_type, std::false_type> multi_edge{};
  constexpr settable_prop<struct compressed_t, std::true_type, std::false_type> compressed{};
} // namespace collect_props

namespace state_machine {

  template <class CRTP, class... States>
  struct any_of {
    template <class T, class Props = properties::properties<>>
    constexpr static auto valid() {
      if constexpr ((static_cast<bool>(States::template valid<T, Props>()) + ...) == 1) {
        return std::tuple_element_t<
              0, decltype(std::tuple_cat(
                       std::declval<std::conditional_t<States::template valid<T, Props>(), std::tuple<States>,
                                                       std::tuple<>>>()...))>::template valid<T, Props>();
      } else if constexpr ((static_cast<bool>(States::template valid<T, Props>()) + ...) > 1) {
        return assert_types::reason<
              multiple_good_recognition, assert_types::input<T, Props>, assert_types::at<CRTP>,
              decltype(std::tuple_cat(
                    std::declval<std::conditional_t<States::template valid<T, Props>(),
                                                    std::tuple<decltype(States::template valid<T, Props>())>,
                                                    std::tuple<>>>()...))>{};
      } else if constexpr ((static_cast<bool>(States::template valid_type<T>()) + ...) == 1) {
        return std::tuple_element_t<
              0, decltype(std::tuple_cat(
                       std::declval<std::conditional_t<States::template valid_type<T>(), std::tuple<States>,
                                                       std::tuple<>>>()...))>::template valid<T, Props>();
      } else if constexpr ((static_cast<bool>(States::template valid_type<T>()) + ...) > 1 &&
                           (static_cast<bool>(States::template valid_conditions<T, Props>()) + ...) == 1) {
        return std::tuple_element_t<
              0, decltype(std::tuple_cat(std::declval<std::conditional_t<States::template valid_conditions<T, Props>(),
                                                                         std::tuple<States>, std::tuple<>>>()...))>::
              template valid<T, Props>();
      } else if constexpr ((static_cast<bool>(States::template valid_conditions<T, Props>()) + ...) > 1) {
        return assert_types::reason<
              no_good_recognition, assert_types::input<T, Props>, assert_types::at<CRTP>,
              assert_types::got<false,
                                decltype(std::tuple_cat(std::declval<std::conditional_t<
                                                              States::template valid_conditions<T>(),
                                                              std::tuple<decltype(States::template valid<T, Props>())>,
                                                              std::tuple<>>>()...))>>{};
      } else if constexpr ((static_cast<bool>(States::template valid_type<T>()) + ...) > 1) {
        return assert_types::reason<
              no_good_recognition, assert_types::input<T, Props>, assert_types::at<CRTP>,
              assert_types::got<false,
                                decltype(std::tuple_cat(std::declval<std::conditional_t<
                                                              States::template valid_type<T>(),
                                                              std::tuple<decltype(States::template valid<T, Props>())>,
                                                              std::tuple<>>>()...))>>{};
      } else {
        return assert_types::reason<no_good_recognition, assert_types::input<T, Props>, assert_types::at<CRTP>>{};
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

  template <class, class Props>
  struct applier_t {
    constexpr static valid_props<Props> value{};
  };

  template <class Fail>
  struct failed_t {
    constexpr static Fail value{};
  };

  template <class>
  struct to_apply_t {};

  template <class T, class Props>
  constexpr static applier_t<T, Props> applier{};

  template <class State>
  constexpr static to_apply_t<State> to_apply{};

  template <class T, class Props, class State>
  constexpr auto operator>>(applier_t<T, Props> const&, to_apply_t<State> const&) {
    if constexpr (State::template valid<T, Props>()) {
      return applier_t<T, typename decltype(State::template valid<T, Props>())::properties>{};
    } else {
      return failed_t<decltype(State::template valid<T, Props>())>{};
    }
  }

  template <class Fail, class State>
  constexpr failed_t<Fail> operator>>(failed_t<Fail> const&, to_apply_t<State> const&) {
    return {};
  }

  template <class TypeFilter = any, auto* Conditions = no, auto* Properties = empty, class... NextStates>
  struct transition {
    template <class T, class Props = properties::properties<>>
    constexpr static auto valid() {
      if constexpr (!valid_type<T>()) {
        return TypeFilter::template valid<T>();
      } else if constexpr (!valid_conditions<T, Props>()) {
        return Conditions->template valid<T, Props>();
      } else {
        return (applier<T, add_properties<Props, Properties, T>> >> ... >> to_apply<NextStates>).value;
      }
    }

    template <class T>
    constexpr static auto valid_type() {
      return TypeFilter::template valid<T>();
    }

    template <class T, class Props = properties::properties<>, std::enable_if_t<valid_type<T>()>* = nullptr>
    constexpr static auto valid_conditions() {
      return Conditions->template valid<T, Props>();
    }

    template <class T, class Props = properties::properties<>, std::enable_if_t<!valid_type<T>()>* = nullptr>
    constexpr static auto valid_conditions() {
      return valid_type<T>();
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


  template <template <class> class Tr>
  struct node
        : any_of_r<node<Tr>,
                   Tr,
                   transition<index, user_node == f && node_type<>, &node_type<>>,
                   transition<any, user_node == t && node_type<>>> {};

  template <template <class> class Tr = T>
  struct edge
        : any_of_r<edge<Tr>,
                   Tr,
                   transition<index, user_edge == f && edge_type<>, &edge_type<>>,
                   transition<any, user_edge == t && edge_type<>>,
                   transition<any, user_edge == na && exclude<edge_prop_t> && not_range<>, &edge_prop<>>> {};

  template <template <class> class Tr = T>
  struct adj_list_cont
        : any_of_r<adj_list_cont<Tr>,
                   Tr,
                   transition<range, no, empty, node<E>>,
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
  struct node_pair : transform<Tr, transition<tuple_eq_2, no, empty, node<tup_0>, node<tup_N_1>>> {};

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
                 simple_transition<edge_list_n>> {};

  template <template <class> class Tr = T>
  struct graph_ep
        : any_of_r<graph_ep<Tr>,
                   Tr,
                   transition<any, no, &(type = adj_list_v), adj_list>,
                   transition<any, no, &(type = adj_mat_v), adj_mat>,
                   transition<any, no, &(type = edge_list_v), edge_list>> {};

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
