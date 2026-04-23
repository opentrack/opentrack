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
    int gameid, gameid2;
    unsigned char table[8];
    bool stop;
    // Monotonic counter incremented by the consumer (e.g. the
    // opentrack.xpl X-Plane plugin) when the user requests a
    // re-center. opentrack's proto-wine reader compares this against
    // its last-seen value on each pose() tick and fires a centering
    // request on increment. Used to let a sim-side button (like a
    // Honeycomb yoke button bound to an X-Plane command) recenter
    // opentrack without needing the game's keystrokes to survive
    // its own focus-grabbing - Carbon global hotkeys don't fire when
    // X-Plane has exclusive focus on macOS, so keyboard-based
    // shortcut paths (Enjoyable remapping to F-keys, etc.) don't
    // work mid-flight.
    //
    // Defined at the end of the struct so older consumers that only
    // know the original layout (first 4 fields) still read/write the
    // same offsets for pose data - backward-compatible extension.
    int center_seq;
};
