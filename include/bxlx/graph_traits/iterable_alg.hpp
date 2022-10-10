//
// Copyright (c) 2022-2022 Bela Schaum (schaumb at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BXLX_GRAPH_TRAITS_ITERABLE_ALG_HPP
#define BXLX_GRAPH_TRAITS_ITERABLE_ALG_HPP

namespace bxlx::utils {
    struct default_sentinel_t {} constexpr sentinel;

    template<class impl_t, class value_t>
    struct iterable {
        struct const_iterator {

            [[nodiscard]] constexpr bool operator !=(default_sentinel_t) const {
                return impl != nullptr;
            }

            [[nodiscard]] constexpr const value_t& operator*() const {
                return *res;
            }

            [[nodiscard]] constexpr const value_t* operator->() const {
                return res;
            }

            constexpr const_iterator& operator++() {
                impl->next();
                if (impl->has_end()) {
                    this->impl = nullptr;
                } else {
                    res = &impl->get_current();
                }
                return *this;
            }

            impl_t* impl;
            const value_t* res;
        };

        [[nodiscard]] constexpr const_iterator begin() {
            auto this_ = static_cast<impl_t*>(this);
            if (this_->has_end())
                return {nullptr};
            return {this_, &this_->get_current()};
        }

        [[nodiscard]] constexpr default_sentinel_t end() const {
            return sentinel;
        }
    };
}

#endif //BXLX_GRAPH_TRAITS_ITERABLE_ALG_HPP
