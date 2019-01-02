#pragma once

#include <cstdlib> // for EXIT_SUCCESS, EXIT_FAILRUE

#ifndef _WIN32
#   include <sysexits.h>
#else
// this conforms to BSD sysexits(3)
// reference the manual page on FreeBSD or Linux for semantics
#   define EX_OK          0
#   define EX_USAGE       64
#   define EX_DATAERR     65
#   define EX_NOINPUT     66
#   define EX_NOUSER      67
#   define EX_NOHOST      68
#   define EX_UNAVAILABLE 69
#   define EX_SOFTWARE    70
#   define EX_OSERR       71
#   define EX_OSFILE      72
#   define EX_CANTCREAT   73
#   define EX_IOERR       74
#   define EX_TEMPFAIL    75
#   define EX_PROTOCOL    76
#   define EX_NOPERM      77
#   define EX_CONFIG      78
#endif


