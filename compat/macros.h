#pragma once

#if defined _MSC_VER
#   define tr_never_inline __declspec(noinline)
#else
#   define tr_never_inline __attribute__((noinline))
#endif

#if defined _MSC_VER
#   define tr_force_inline __forceinline
#else
#   define tr_force_inline __attribute__((always_inline)) inline
#endif

#if defined _MSC_VER
#   define tr_function_name __FUNCSIG__
#else
#   define tr_function_name __PRETTY_FUNCTION__
#endif

#ifdef _MSC_VER
#   define tr_unreachable() do { __assume(0); *(volatile int*)nullptr = 0; } while (0) /* NOLINT(clang-analyzer-core.NullDereference) */
#else
#   define tr_unreachable() do { __builtin_unreachable(); *(volatile int*)nullptr = 0; } while (0) /* NOLINT(clang-analyzer-core.NullDereference) */
#endif

#ifdef __cplusplus
#   define progn(...) ([&]() -> decltype(auto) { __VA_ARGS__ }())
#   define eval_once2(expr, ctr) eval_once3(expr, ctr)
#   define eval_once3(expr, ctr) ([&] { [[maybe_unused]] static const char init_ ## ctr = ((void)(expr), 0); }())
#   ifdef QT_NO_DEBUG_OUTPUT
#       define eval_once(expr) void()
#   else
#       define eval_once(expr) eval_once2(expr, __COUNTER__)
#   endif

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
