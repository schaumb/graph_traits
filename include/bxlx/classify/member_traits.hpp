//
// Copyright (c) 2022-2023 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_MEMBER_TRAITS_HPP
#define BXLX_GRAPH_MEMBER_TRAITS_HPP

#include <type_traits>

namespace bxlx::graph::type_traits::detail {

template <class Type, class ResDed, class = void, class... Args>
struct member_function_invoke_result {
  constexpr static bool is_const  = std::is_const_v<std::remove_reference_t<Type>>;
  constexpr static bool is_lv_ref = std::is_lvalue_reference_v<Type>;
  constexpr static bool is_rv_ref = std::is_rvalue_reference_v<Type>;
  template <class Res = ResDed, class T = Type>
  auto operator()(Res (T::*)(Args...)) -> std::enable_if_t<!is_const, Res>;
  template <class Res = ResDed, class T = Type>
  auto operator()(Res (T::*)(Args...) const) const -> Res;
  template <class Res = ResDed, class T = Type>
  auto operator()(Res (T::*)(Args...) &) -> std::enable_if_t<!is_rv_ref, Res>;
  template <class Res = ResDed, class T = Type>
  auto operator()(Res (T::*)(Args...) const&) const -> Res;
  template <class Res = ResDed, class T = Type>
  auto operator()(Res (T::*)(Args...) &&) const volatile -> std::enable_if_t<!is_lv_ref, Res>;
  // volatile and const&& overloads are not handled by design.
};

template <class Type, class ResDed, class... Args>
struct member_function_invoke_result<Type, ResDed, std::enable_if_t<(std::is_void_v<Args> || ...)>, Args...> {};


template <class T, class Res = void, class... Args>
inline member_function_invoke_result<T, Res, void, Args...> member_function_invoke_result_v{};


namespace class_member_traits {
  template <class Range>
  constexpr inline bool can_inspect_member = std::is_class_v<Range>;

#define HAS_MEMBER_FUNCTION_TEMPLATE_VARARG(name)                                                                      \
  template <class Range, class... Args>                                                                                \
  using get_##name##_result_t =                                                                                        \
        decltype(member_function_invoke_result_v<Range, void, Args&&...>(&Range::template name<Args&&...>));           \
  template <class Range, bool = can_inspect_member<Range>, class = void, class...>                                     \
  constexpr inline bool has_##name = false;                                                                            \
  template <class Range, class... Args>                                                                                \
  constexpr inline bool has_##name<Range, true, std::void_t<get_##name##_result_t<Range, Args...>>, Args...> = true;   \
  template <class Range, class... Args>                                                                                \
  constexpr inline bool has_##name##_v = has_##name<Range, can_inspect_member<Range>, void, Args...>;

  HAS_MEMBER_FUNCTION_TEMPLATE_VARARG(emplace_back)
  HAS_MEMBER_FUNCTION_TEMPLATE_VARARG(emplace_front)
  HAS_MEMBER_FUNCTION_TEMPLATE_VARARG(emplace)
#undef HAS_MEMBER_FUNCTION_TEMPLATE_VARARG

#define HAS_MEMBER_FUNCTION_RES_VARARG(name)                                                                           \
  template <class Range, class ResHelper, bool, class, class...>                                                       \
  constexpr inline bool has_##name_res = false;                                                                        \
  template <class Range, class ResHelper, class... Args>                                                               \
  constexpr inline bool has_##name_res<                                                                                \
        Range, ResHelper, true,                                                                                        \
        std::void_t<decltype(member_function_invoke_result_v<Range, ResHelper, Args...>(&Range::name))>, Args...> =    \
        true;                                                                                                          \
  template <class Range, class ResHelper, class... Args>                                                               \
  constexpr inline bool has_##name##_res_v =                                                                           \
        has_##name_res<Range, ResHelper, can_inspect_member<Range>, void, Args...>;

  HAS_MEMBER_FUNCTION_RES_VARARG(equal_range)

#undef HAS_MEMBER_FUNCTION_RES_VARARG

#define HAS_MEMBER_FUNCTION_VARARG(name)                                                                               \
  template <class Range, class... Args>                                                                                \
  using get_##name##_result_t = decltype(member_function_invoke_result_v<Range, void, Args...>(&Range::name));         \
  template <class Range, bool = can_inspect_member<Range>, class = void, class...>                                     \
  constexpr inline bool has_##name = false;                                                                            \
  template <class Range, class... Args>                                                                                \
  constexpr inline bool has_##name<Range, true, std::void_t<get_##name##_result_t<Range, Args...>>, Args...> = true;   \
  template <class Range, class... Args>                                                                                \
  constexpr inline bool has_##name##_v = has_##name<Range, can_inspect_member<Range>, void, Args...>;

  HAS_MEMBER_FUNCTION_VARARG(erase)
  HAS_MEMBER_FUNCTION_VARARG(erase_after)
  HAS_MEMBER_FUNCTION_VARARG(at)
  HAS_MEMBER_FUNCTION_VARARG(insert)
  HAS_MEMBER_FUNCTION_VARARG(insert_after)
  HAS_MEMBER_FUNCTION_VARARG(push_back)
  HAS_MEMBER_FUNCTION_VARARG(push_front)
#undef HAS_MEMBER_FUNCTION_VARARG

#define HAS_MEMBER_FUNCTION_NO_ARG(name)                                                                               \
  template <class Range>                                                                                               \
  using get_##name##_result_t = decltype(member_function_invoke_result_v<Range>(&Range::name));                        \
  template <class Range, bool = can_inspect_member<Range>, class = void>                                               \
  constexpr inline bool has_##name##_v = false;                                                                        \
  template <class Range>                                                                                               \
  constexpr inline bool has_##name##_v<Range, true, std::void_t<get_##name##_result_t<Range>>> = true;

  HAS_MEMBER_FUNCTION_NO_ARG(before_begin)
  HAS_MEMBER_FUNCTION_NO_ARG(data)
  HAS_MEMBER_FUNCTION_NO_ARG(length)
  HAS_MEMBER_FUNCTION_NO_ARG(key_comp)
  HAS_MEMBER_FUNCTION_NO_ARG(key_eq)
  HAS_MEMBER_FUNCTION_NO_ARG(hash_function)
  HAS_MEMBER_FUNCTION_NO_ARG(extract)
#undef HAS_MEMBER_FUNCTION_NO_ARG

#define HAS_MEMBER_OPERATOR_NO_ARG(name, op)                                                                           \
  template <class Range>                                                                                               \
  using get_##name##_result_t =                                                                                        \
        decltype(member_function_invoke_result_v<Range>(&std::remove_reference_t<Range>::operator op));                \
  template <class Range, bool = can_inspect_member<std::remove_reference_t<Range>>, class = void>                      \
  constexpr inline bool has_##name##_v = false;                                                                        \
  template <class Range>                                                                                               \
  constexpr inline bool has_##name##_v<Range, true, std::void_t<get_##name##_result_t<Range>>> = true;

  HAS_MEMBER_OPERATOR_NO_ARG(star_op, *)
  HAS_MEMBER_OPERATOR_NO_ARG(increment_op, ++)
  HAS_MEMBER_OPERATOR_NO_ARG(decrement_op, --)
#undef HAS_MEMBER_OPERATOR_NO_ARG


#define HAS_MEMBER_OPERATOR_ONE_ARG(name, op)                                                                          \
  template <class Range, class Arg>                                                                                    \
  using get_##name##_result_t =                                                                                        \
        decltype(member_function_invoke_result_v<Range, void, Arg>(&std::remove_reference_t<Range>::operator op));     \
  template <class Range, class Arg, bool = can_inspect_member<std::remove_reference_t<Range>>, class = void>           \
  constexpr inline bool has_##name##_v = false;                                                                        \
  template <class Range, class Arg>                                                                                    \
  constexpr inline bool has_##name##_v<Range, Arg, true, std::void_t<get_##name##_result_t<Range, Arg>>> = true;

  HAS_MEMBER_OPERATOR_ONE_ARG(subscript_op, [])

#undef HAS_MEMBER_OPERATOR_ONE_ARG

#define HAS_MEMBER_TYPE(name)                                                                                          \
  template <class Range>                                                                                               \
  using get_##name##_member_t = typename Range::name;                                                                  \
  template <class Range, bool = can_inspect_member<Range>, class = void>                                               \
  constexpr inline bool has_##name##_type_v = false;                                                                   \
  template <class Range>                                                                                               \
  constexpr inline bool has_##name##_type_v<Range, true, std::void_t<get_##name##_member_t<Range>>> = true;

  HAS_MEMBER_TYPE(key_type)
  HAS_MEMBER_TYPE(value_type)
  HAS_MEMBER_TYPE(size_type)
  HAS_MEMBER_TYPE(key_compare)
  HAS_MEMBER_TYPE(key_equal)
  HAS_MEMBER_TYPE(hasher)
  HAS_MEMBER_TYPE(container_type)
#undef HAS_MEMBER_TYPE

#define TYPE_OR_GETTER(type_name, getter)                                                                              \
  template <class Range, class Default, bool = has_##type_name##_type_v<Range>, bool = has_##getter##_v<Range>>        \
  struct type_name {                                                                                                   \
    using type = Default;                                                                                              \
  };                                                                                                                   \
  template <class Range, class Default, bool any>                                                                      \
  struct type_name<Range, Default, true, any> {                                                                        \
    using type = get_##type_name##_member_t<Range>;                                                                    \
  };                                                                                                                   \
  template <class Range, class Default>                                                                                \
  struct type_name<Range, Default, false, true> {                                                                      \
    using type = get_##getter##_result_t<Range>;                                                                       \
  };                                                                                                                   \
  template <class Range, class Default = void>                                                                         \
  using type_name##_t = typename type_name<Range, Default>::type;

  TYPE_OR_GETTER(key_compare, key_comp)
  TYPE_OR_GETTER(key_equal, key_eq)
  TYPE_OR_GETTER(hasher, hash_function)
#undef TYPE_OR_GETTER

  template <class Range, class Op, bool = can_inspect_member<std::remove_reference_t<Range>>, class = void>
  constexpr inline bool has_conversion_v = false;
  template <class Range, class Op>
  constexpr inline bool
        has_conversion_v<Range,
                         Op,
                         true,
                         std::void_t<decltype(member_function_invoke_result_v<Range>(&Range::operator Op))>> = true;

  template <class, class, bool, class = void, class...>
  constexpr inline bool has_try_emplace = false;
  template <class Range, class KeyType, class... Args>
  constexpr inline bool
        has_try_emplace<Range,
                        KeyType,
                        true,
                        std::void_t<decltype(member_function_invoke_result_v<Range, void, KeyType, Args&&...>(
                              &Range::template try_emplace<Args&&...>))>,
                        Args...> = true;

  template <class Range, class KeyType = const get_key_type_member_t<Range>&, class... Args>
  constexpr inline bool has_try_emplace_v = has_try_emplace<Range, KeyType, can_inspect_member<Range>, void, Args...>;

  template <class Range, class = get_size_type_member_t<Range>, bool = can_inspect_member<Range>, class = void>
  constexpr inline bool has_resize_v = false;
  template <class Range, class SizeType>
  constexpr inline bool
        has_resize_v<Range,
                     SizeType,
                     true,
                     std::void_t<decltype(member_function_invoke_result_v<Range, void, SizeType>(&Range::resize))>> =
              true;
} // namespace class_member_traits
} // namespace bxlx::graph::type_traits::detail

#endif //BXLX_GRAPH_MEMBER_TRAITS_HPP
