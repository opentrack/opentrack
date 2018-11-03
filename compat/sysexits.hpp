#pragma once

#include <cstdlib> // for EXIT_SUCCESS, EXIT_FAILRUE

/* FreeBSD sysexits(3)
 *
 * The input data was incorrect	in some	way.  This
 * should only be used for user's data and not system
 * files.
 */

#if !defined EX_OSFILE
#   define EX_OSFILE 72
#endif
