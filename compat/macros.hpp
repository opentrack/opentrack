#pragma once

#if defined _MSC_VER
#   define MEMORY_BARRIER() _ReadWriteBarrier()
#else
#   define MEMORY_BARRIER() asm volatile("" ::: "memory")
#endif

#if defined _MSC_VER
#   define never_inline __declspec(noinline)
#elif defined __GNUG__
#   define never_inline __attribute__((noinline))
#else
#   define never_inline
#endif

#if defined __cplusplus
#   define restrict_ptr __restrict
#endif

#if defined _MSC_VER
#   define force_inline __forceinline
#else
#   define force_inline __attribute__((always_inline, gnu_inline)) inline
#endif

#ifdef Q_CREATOR_RUN
#   define warn_result_unused
#elif defined _MSC_VER
#   define warn_result_unused _Check_return_
#else
#   define warn_result_unused __attribute__((warn_unused_result))
#endif

#if defined __GNUC__
#   define likely(x)       __builtin_expect(!!(x),1)
#   define unlikely(x)     __builtin_expect(!!(x),0)
#else
#   define likely(x) (x)
#   define unlikely(x) (x)
#endif

#if defined _MSC_VER
#   define OTR_FUNNAME (__FUNCSIG__)
#else
#   define OTR_FUNNAME (__PRETTY_FUNCTION__)
#endif

#if !defined PP_CAT
#   define PP_CAT(x,y) PP_CAT1(x,y)
#   define PP_CAT1(x,y) x##y
#endif

#if defined __cplusplus

// from now only C++ macros

#if defined _MSC_VER
#   define OTR_DEPRECATE(msg, decl, body) __declspec(deprecated(msg)) decl body
#else
#   define OTR_DEPRECATE(msg, decl, body) decl body __attribute__((deprecated(msg)))
#endif

namespace static_warning_detail {
    template<bool> struct test___132;

    template<>
    struct test___132<true>
    {
        static constexpr inline void check() {}
    };
} // ns static_warning_detail

#define static_warning_template(cond, msg)                                      \
    {                                                                           \
        template<bool>                                                          \
        struct ::static_warning_detail::test___132                              \
        {                                                                       \
            OTR_DEPRECATE(msg, static constexpr inline void check(), {})        \
        };                                                                      \
        ::static_warning_detail::test___132<(cond)>::check();                   \
    }

#define static_warning(cond, msg)                                               \
    static_warning_template(cond, PP_CAT(msg, PP_CAT("\nExpression: ", #cond)))

#endif
