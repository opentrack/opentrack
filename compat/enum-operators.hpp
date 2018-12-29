#pragma once

#include <type_traits>

#define OTR_FLAGS_OP2(type, op)                                         \
    inline type operator op (type a, type b)                            \
    {                                                                   \
        using t_ = std::underlying_type_t<type>;                        \
        return type(t_((a)) op t_((b)));                                \
    } // end

#define OTR_FLAGS_DELETE_SHIFT(type, op)                                \
    template<typename u>                                                \
    type operator op (type, u) = delete // end

#define OTR_FLAGS_OP1(type, op)                                         \
    inline type operator op (type x)                                    \
    {                                                                   \
        using t_ = std::underlying_type_t<type>;                        \
        return type(op t_((x)));                                        \
    } // end

#define DEFINE_ENUM_OPERATORS(type)                                     \
    OTR_FLAGS_OP2(type, |)                                              \
    OTR_FLAGS_OP2(type, &)                                              \
    OTR_FLAGS_OP2(type, ^)                                              \
    OTR_FLAGS_OP1(type, ~)                                              \
    OTR_FLAGS_SHIFT(type, <<);                                          \
    OTR_FLAGS_SHIFT(type, >>) // end
