#pragma once

#include <cassert>
#include <cstring>

namespace thorin {

/// A bitcast.
template<class D, class S>
inline D bitcast(const S& src) {
    D dst;
    auto s = reinterpret_cast<const void*>(&src);
    auto d = reinterpret_cast<void*>(&dst);

    if constexpr (sizeof(D) == sizeof(S)) memcpy(d, s, sizeof(D));
    if constexpr (sizeof(D) < sizeof(S)) memcpy(d, s, sizeof(D));
    if constexpr (sizeof(D) > sizeof(S)) {
        memset(d, 0, sizeof(D));
        memcpy(d, s, sizeof(S));
    }
    return dst;
}

namespace detail {
template<class T>
concept Nodeable = requires {
    T::Node;
};
}

template<class B>
class RuntimeCast {
public:
    /// `dynamic_cast`.
    template<class T>
    T* isa() {
        if constexpr (detail::Nodeable<T>) {
            return static_cast<B*>(this)->node() == T::Node ? static_cast<T*>(this) : nullptr;
        } else {
            return dynamic_cast<T*>(static_cast<B*>(this));
        }
    }

    // clang-format off
    /// `static_cast` with debug check.
    template<class T> T* as() { assert(isa<T>()); return  static_cast<T*>(this); }

    /// Yields `B*` if it is *either* @p T or @p U and `nullptr* otherwise.
    template<class T, class U> B* isa() { return (isa<T>() || isa<U>()) ? static_cast<B*>(this) : nullptr; }

    template<class T         > const T* isa() const { return const_cast<RuntimeCast*>(this)->template isa<T   >(); } ///< `const` version.
    template<class T         > const T*  as() const { return const_cast<RuntimeCast*>(this)->template isa<T   >(); } ///< `const` version.
    template<class T, class U> const B* isa() const { return const_cast<RuntimeCast*>(this)->template isa<T, U>(); } ///< `const` version.
    // clang-format on
};

} // namespace thorin
