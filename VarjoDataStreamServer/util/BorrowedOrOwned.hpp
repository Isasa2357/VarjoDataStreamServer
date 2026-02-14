#pragma once

#include <variant>
#include <functional>
#include <utility>
#include <cassert>

template<class T>
class BorrowedOrOwned {
    std::variant<std::reference_wrapper<const T>, T> v_;

public:
    BorrowedOrOwned(const T& t) : v_(std::cref(t)) 
    {}

    BorrowedOrOwned(T&& t) : v_(std::move(t)) 
    {}

    const T& view() const {
        if (auto p = std::get_if<std::reference_wrapper<const T>>(&v_)) return p->get();
        return std::get<T>(v_);
    }

    bool owns() const noexcept { return std::holds_alternative<T>(v_); }

    T materialize()&& {
        if (owns()) return std::get<T>(std::move(v_));
        return T(view());
    }
};