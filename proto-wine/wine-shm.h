#pragma once

#define WINE_SHM_NAME "facetracknoir-wine-shm"
#define WINE_MTX_NAME "facetracknoir-wine-mtx"

// OSX sdk 10.8 build error otherwise
#undef _LIBCPP_MSVCRT

#include <memory>

template<typename t> using ptr = std::shared_ptr<t>;

struct WineSHM {
    double data[6];
    int gameid, gameid2;
    unsigned char table[8];
    bool stop;
};
