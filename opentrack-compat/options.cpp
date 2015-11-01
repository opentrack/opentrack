#include "options.hpp"

namespace options
{

namespace detail
{
OPENTRACK_COMPAT_EXPORT opt_singleton& singleton()
{
    static auto ret = std::make_shared<opt_singleton>();
    return *ret;
}

}

}
