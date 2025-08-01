#pragma once

#if __cplusplus < 202002L
#include <atomic>
#include <thread>
#include <type_traits>
#include <climits>

namespace sgcl::detail {
    // minimal std::strong_ordering replacement
    enum class strong_ordering { less = -1, equal = 0, greater = 1 };

    struct compare_three_way {
        template<class L, class R>
        constexpr strong_ordering operator()(const L& l, const R& r) const {
            if (l < r) return strong_ordering::less;
            if (r < l) return strong_ordering::greater;
            return strong_ordering::equal;
        }
    };

    template<class T, class U = T>
    using compare_three_way_result_t = strong_ordering;

    template<class It1, class It2, class Comp>
    constexpr strong_ordering lexicographical_compare_three_way(
        It1 f1, It1 l1, It2 f2, It2 l2, Comp comp) {
        for (; f1 != l1 && f2 != l2; ++f1, ++f2) {
            auto c = comp(*f1, *f2);
            if (c != strong_ordering::equal) return c;
        }
        if (f1 == l1 && f2 == l2) return strong_ordering::equal;
        return (f1 == l1) ? strong_ordering::less : strong_ordering::greater;
    }

    // basic spin-wait implementation for atomic wait/notify
    template<class Atomic, class T>
    void atomic_wait(Atomic& a, T old) noexcept {
        while (a.load(std::memory_order_acquire) == old) {
            std::this_thread::yield();
        }
    }

    template<class T>
    constexpr int countr_zero(T v) noexcept {
        int c = 0;
        while (v && !(v & 1)) {
            v >>= 1;
            ++c;
        }
        return c;
    }

    template<class Atomic>
    void atomic_notify_one(Atomic&) noexcept {
    }

    template<class Atomic>
    void atomic_notify_all(Atomic&) noexcept {
    }
}

#else
#include <compare>
namespace sgcl::detail {
    using std::strong_ordering;
    using std::compare_three_way;
    using std::lexicographical_compare_three_way;
    template<class T, class U = T>
    using compare_three_way_result_t = std::compare_three_way_result_t<T, U>;
    using std::countr_zero;
    template<class Atomic, class T>
    inline void atomic_wait(Atomic& a, T old) noexcept { a.wait(old); }
    template<class Atomic>
    inline void atomic_notify_one(Atomic& a) noexcept { a.notify_one(); }
    template<class Atomic>
    inline void atomic_notify_all(Atomic& a) noexcept { a.notify_all(); }
}
#endif
