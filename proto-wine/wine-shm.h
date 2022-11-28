#pragma once

#define WINE_SHM_NAME "facetracknoir-wine-shm"
#define WINE_MTX_NAME "facetracknoir-wine-mtx"

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

struct WineSHM {
    double data[6];
    int gameid, gameid2; //gameid one is passed to us by FreeTrack. No idea about gameid2. That thing is just mirrored back in this module to FreeTrack.
    unsigned char table[8];
    bool stop;
    //char message[1024];
};
