#pragma once

#define WINE_SHM_NAME "facetracknoir-wine-shm"
#define WINE_MTX_NAME "facetracknoir-wine-mtx"

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif

// OSX sdk 10.8 build error otherwise
#undef _LIBCPP_MSVCRT

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

#include <memory>

template<typename t> using ptr = std::shared_ptr<t>;

struct WineSHM {
    double data[6];
    int gameid, gameid2;
    unsigned char table[8];
    bool stop;
};
