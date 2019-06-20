#pragma once

#include <type_traits>

#define OTR_FLAGS_OP2(type, op)                                         \
    constexpr inline type operator op (type a, type b)                  \
    {                                                                   \
        using t_ = std::underlying_type_t<type>;                        \
        return type(t_((a)) op t_((b)));                                \
    } // end

#define OTR_FLAGS_DELETE_SHIFT(type, op)                                \
    template<typename u>                                                \
    type operator op (type, u) = delete // end

#define OTR_FLAGS_OP1(type, op)                                         \
    constexpr inline type operator op (type x)                          \
    {                                                                   \
        using t_ = std::underlying_type_t<type>;                        \
        return type(op t_((x)));                                        \
    } // end

#define OTR_FLAGS_ASSIGN_OP(type, op)                                   \
    constexpr inline type& operator op ## = (type& lhs, type rhs)       \
    {                                                                   \
        using t_ = std::underlying_type_t<decltype(rhs)>;               \
        lhs = type(t_((lhs)) op t_((rhs)));                             \
        return lhs;                                                     \
    } //end

#define OTR_FLAGS_DELETE_SHIFT_ASSIGN(type, op)                         \
    type operator op ## = (type& lhs, type rhs) = delete //end

#define DEFINE_ENUM_OPERATORS(type)                                     \
    OTR_FLAGS_OP2(type, |)                                              \
    OTR_FLAGS_OP2(type, &)                                              \
    OTR_FLAGS_OP2(type, ^)                                              \
    OTR_FLAGS_OP1(type, ~)                                              \
    OTR_FLAGS_DELETE_SHIFT(type, <<);                                   \
    OTR_FLAGS_DELETE_SHIFT(type, >>);                                   \
    OTR_FLAGS_ASSIGN_OP(type, |)                                        \
    OTR_FLAGS_ASSIGN_OP(type, &)                                        \
    OTR_FLAGS_ASSIGN_OP(type, ^)                                        \
    OTR_FLAGS_DELETE_SHIFT_ASSIGN(type, <<);                            \
    OTR_FLAGS_DELETE_SHIFT_ASSIGN(type, >>) // end
