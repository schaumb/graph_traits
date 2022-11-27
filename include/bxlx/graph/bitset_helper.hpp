//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_BITSET_HELPER_HPP
#define BXLX_GRAPH_TRAITS_BITSET_HELPER_HPP

#include <utility>
#include <cstdint>
#include <limits>
#include <functional>
#include "traits/type_classification.hpp"

template<class TT, class T = std::remove_reference_t<TT>, bool = bxlx::detail2::is_bitset_like_v<T>>
struct bitset_like_iterator_helper {
    template<class U = T, class V = decltype(std::declval<const U&>()._Getword(std::size_t{}))>
    static constexpr std::integral_constant<V (U::*) (std::size_t) const, &U::_Getword> get_word_impl(std::nullptr_t);

    template<class U = T, class V = decltype(std::declval<const U&>().begin()._M_p[std::size_t{}])>
    static constexpr auto get_word_impl(std::nullptr_t) {
        constexpr auto get_underlying_data = [] (const U& c, std::size_t v) -> V { return c.begin()._M_p[v]; };
        return std::integral_constant<V (*)(const U&, std::size_t), +get_underlying_data>{};
    }

    template<class U = T, class V = decltype(std::declval<const U&>()._Myvec[std::size_t{}])>
    static constexpr auto get_word_impl(std::nullptr_t, int = 0) {
        constexpr auto get_underlying_data = [] (const U& c, std::size_t v) -> V { return c._Myvec[v]; };
        return std::integral_constant<V (*)(const U&, std::size_t), +get_underlying_data>{};
    }

    template<class U = T> static constexpr std::integral_constant<void (U::*) (std::size_t) const, nullptr> get_word_impl(...);

    template<class U = T, class V = decltype(std::declval<const U&>()._Find_first())>
    static constexpr std::integral_constant<V (U::*) () const, &U::_Find_first> get_first_impl(std::nullptr_t);
    template<class U = T> static constexpr std::nullptr_t get_first_impl(...);

    template<class U = T, class V = decltype(std::declval<const U&>()._Find_next(std::size_t{}))>
    static constexpr std::integral_constant<V (U::*) (std::size_t) const, &U::_Find_next> get_next_impl(std::nullptr_t);
    template<class U = T> static constexpr std::nullptr_t get_next_impl(...);

    template<class U = std::uint64_t, class = decltype(__builtin_ctzll(std::declval<U>()))>
    static constexpr auto get_builtin_countr_zero(std::nullptr_t) {
        constexpr auto builtin_fun_wrapper = [] (U v) -> int { return v ? __builtin_ctzll(v) : std::numeric_limits<U>::digits; };
        return std::integral_constant<int (*)(U), +builtin_fun_wrapper>{};
    };

    template<class U = std::uint64_t, class = decltype(_BitScanForward64(std::declval<unsigned long*>(), std::declval<U>()))>
    static constexpr auto get_builtin_countr_zero(std::nullptr_t, int = 0) {
        constexpr auto builtin_fun_wrapper = [] (U v) -> int {
            unsigned long res;
            return _BitScanForward64(&res, v) ? static_cast<int>(res) : std::numeric_limits<U>::digits;
        };
        return std::integral_constant<int (*)(U), +builtin_fun_wrapper>{};
    };

    template<class U = std::uint64_t> static constexpr auto get_builtin_countr_zero(...) {
        constexpr auto fun_wrapper = [] (U x) -> int {
            constexpr unsigned char const mod67[ 67 ] = { 64, 4, 54, 24, 19, 41, 59, 16, 12, 3, 23, 40, 15, 2, 39, 1, 0,
                                                          0, 33, 34, 6, 35, 48, 7, 56, 36, 45, 49, 26, 8, 52, 57, 21,
                                                          37, 31, 46, 43, 50, 29, 27, 61, 9, 63, 53, 18, 58, 11, 22, 14,
                                                          38, 0, 32, 5, 47, 55, 44, 25, 51, 20, 30, 42, 28, 60, 62, 17,
                                                          10, 13 };
            x |= x << 1;
            x |= x << 2;
            x |= x << 4;
            x |= x << 8;
            x |= x << 16;
            x |= x << 32;

            return static_cast<int>(mod67[ x % 67 ]);
        };
        return std::integral_constant<int (*)(U), +fun_wrapper>{};
    };

    constexpr static auto get_word_v = decltype(bitset_like_iterator_helper<T>::template get_word_impl(nullptr)){};
    constexpr static auto get_first_v = decltype(bitset_like_iterator_helper<T>::template get_first_impl(nullptr)){};
    constexpr static auto get_next_v = decltype(bitset_like_iterator_helper<T>::template get_next_impl(nullptr)){};
    constexpr static auto countr_zero = decltype(bitset_like_iterator_helper<T>::template get_builtin_countr_zero(nullptr)){};

    constexpr static std::size_t get_first(const T& obj) {
        if constexpr(get_first_v != nullptr) {
            return (obj.*get_first_v())();
        } else if constexpr (get_word_v != nullptr) {
            using word_t = std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<decltype(get_word_v()), T, std::size_t>>>;
            constexpr auto word_digits = std::numeric_limits<word_t>::digits;
            static_assert(word_digits > 0);
            const auto size_v = obj.size();
            const auto words = size_v / word_digits + (size_v % word_digits > 0);

            for (std::size_t ix{}; ix < words; ++ix)
                if (auto r = std::invoke(get_word_v(), obj, ix))
                    return ix * word_digits + countr_zero(r);

            return size_v;
        } else {
            const auto size_v = obj.size();
            for (std::size_t ix{}; ix < size_v; ++ix)
                if (obj[ix])
                    return ix;

            return size_v;
        }
    }

    constexpr static std::size_t get_next(const T& obj, std::size_t prev) {
        if constexpr(get_next_v != nullptr) {
            return (obj.*get_next_v())(prev);
        } else if constexpr(get_word_v != nullptr) {
            using word_t = std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<decltype(get_word_v()), T, std::size_t>>>;
            constexpr auto word_digits = std::numeric_limits<word_t>::digits;
            const auto size_v = obj.size();
            const auto words = size_v / word_digits + (size_v % word_digits > 0);

            if (++prev >= size_v)
                return size_v;

            std::size_t ix = prev / word_digits;
            if (auto this_word = std::invoke(get_word_v(), obj, ix); this_word &= ~word_t{} << (prev % word_digits))
                return ix * word_digits + countr_zero(this_word);

            for (++ix; ix < words; ++ix)
                if (auto this_word = std::invoke(get_word_v(), obj, ix))
                    return ix * word_digits + countr_zero(this_word);

            return size_v;
        } else {
            const auto size_v = obj.size();
            for (++prev; prev < size_v; ++prev)
                if (obj[prev])
                    return prev;

            return size_v;
        }
    }
    constexpr static std::size_t get_end(const T& obj) {
        return obj.size();
    }

    struct type {
        T& cont;
        constexpr explicit type(T& cont) noexcept : cont(cont) {}

        struct iterator {
            T* cont;
            std::size_t curr;

            constexpr bool operator!=(const iterator& rhs) const {
                return curr != rhs.curr;
            }

            constexpr friend bool operator==(const type& lhs, const type& rhs) {
                return lhs.curr == rhs.curr;
            }

            constexpr auto operator*() const {
                return (*cont)[curr];
            }

            constexpr iterator& operator++() {
                curr = get_next(*cont, curr);
                return *this;
            }
        };

        constexpr auto begin() const {
            return iterator{&cont, get_first(cont)};
        }

        constexpr auto end() const {
            return iterator{&cont, get_end(cont)};
        }
    };
};

template<class T, class TT>
struct bitset_like_iterator_helper<T, TT, false> : std::common_type<T&> {};

template<class T>
using bitset_iterator = typename bitset_like_iterator_helper<T>::type;

#endif //BXLX_GRAPH_TRAITS_BITSET_HELPER_HPP