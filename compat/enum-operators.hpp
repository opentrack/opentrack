#pragma once

#include <type_traits>

#define OTR_FLAGS_OP2(type, op)                                         \
    inline type operator op (type a, type b)                            \
    {                                                                   \
        using t__ = std::underlying_type_t<type>;                       \
        return static_cast<type>(t__((a)) op t__((b)));                 \
    } // end

#define OTR_FLAGS_SHIFT(type, op)                                       \
    type operator op (type, unsigned) = delete

#define OTR_FLAGS_OP1(type, op)                                         \
    inline type operator op (type x)                                    \
    {                                                                   \
        using t__ = std::underlying_type_t<type>;                       \
        return type(op t_((x)));                                        \
    } // end

#define DEFINE_ENUM_OPERATORS(type)                                     \
    OTR_FLAGS_OP2(type, |)                                              \
    OTR_FLAGS_OP2(type, &)                                              \
    OTR_FLAGS_OP2(type, ^)                                              \
    OTR_FLAGS_OP1(type, ~)                                              \
    OTR_FLAGS_SHIFT(type, <<);                                          \
    OTR_FLAGS_SHIFT(type, >>) // end
