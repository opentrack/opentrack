#pragma once

#include <cstdio>

template<unsigned N, unsigned M, typename... xs>
void ps3eye_log(const char (&prefix)[N], const char (&fmt)[M], const xs&... args)
{
    fprintf(stderr, "%s ", prefix);
    fprintf(stderr, fmt, args...);
    if constexpr(M > 1)
        if (fmt[M-2] != '\n')
            fprintf(stderr, "\n");
    fflush(stderr);
}

#define warn(...) ps3eye_log("[ps3eye warn]", __VA_ARGS__)

#define PS3_EYE_DEBUG

#ifdef PS3_EYE_DEBUG
#   define debug(...) ps3eye_log("[ps3eye debug]", __VA_ARGS__)
#   define debug2(...) ps3eye_log("[ps3eye debug2]", __VA_ARGS__)
#else
#   define debug(...) ((void)0)
#   define debug2(...) ((void)0)
#endif
