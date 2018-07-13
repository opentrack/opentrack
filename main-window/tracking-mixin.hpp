#pragma once

#include "mixins.hpp"

#include "logic/work.hpp"

#include <memory>

namespace OTR_MIXIN_NS(tracking_mixin) {

using work_ptr = std::shared_ptr<Work>;

struct OTR_MAIN_EXPORT has_work {
    virtual explicit operator work_ptr() = 0;
    inline has_work() = default;
    virtual ~has_work();
};

}
