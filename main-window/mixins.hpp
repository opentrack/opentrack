#pragma once

#include "export.hpp"

// XXX TODO add is_base_of and void_t stuff

#define OTR_MIXIN_NS(name)  \
    mixins :: detail :: name

#define OTR_DECLARE_MIXIN(name)                                 \
    namespace mixins {                                          \
        using name = :: OTR_MIXIN_NS(name) :: name;             \
    }
