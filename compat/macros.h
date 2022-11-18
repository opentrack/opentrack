#pragma once

#if defined _MSC_VER
#   define never_inline __declspec(noinline)
#else
#   define never_inline __attribute__((noinline))
#endif

#if defined _MSC_VER
#   define force_inline __forceinline
#else
#   define force_inline __attribute__((always_inline))
#endif

#if !defined likely
#   if defined __GNUC__
#      define likely(x)       __builtin_expect(!!(x),1)
#      define unlikely(x)     __builtin_expect(!!(x),0)
#   else
#      define likely(x) (x)
#      define unlikely(x) (x)
#   endif
#endif

#if defined _MSC_VER
#   define function_name __FUNCSIG__
#else
#   define function_name __PRETTY_FUNCTION__
#endif

#ifdef _MSC_VER
#   define unreachable() do { __assume(0); *(volatile int*)nullptr = 0; } while (0) /* NOLINT(clang-analyzer-core.NullDereference) */
#else
#   define unreachable() do { __builtin_unreachable(); *(volatile int*)nullptr = 0; } while (0) /* NOLINT(clang-analyzer-core.NullDereference) */
#endif

#ifdef __cplusplus
#   define progn(...) ([&]() -> decltype(auto) { __VA_ARGS__ }())
#   define eval_once2(expr, ctr) eval_once3(expr, ctr)
#   define eval_once3(expr, ctr) ([&] { [[maybe_unused]] static const char init_ ## ctr = ((void)(expr), 0); }())
#   define eval_once(expr) eval_once2(expr, __COUNTER__)

#define OTR_DISABLE_MOVE_COPY(type)         \
    type(const type&) = delete;             \
    type(type&&) = delete;                  \
    type& operator=(const type&) = delete;  \
    type& operator=(type&&) = delete
#endif

#ifdef _MSC_VER
#   define strncasecmp _strnicmp
#   define strcasecmp _stricmp
#endif
