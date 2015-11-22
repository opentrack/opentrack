#include "options.hpp"

namespace options
{

namespace detail
{
OPENTRACK_COMPAT_EXPORT opt_singleton& singleton()
{
    static opt_singleton ret;
    return ret;
}

}

}
