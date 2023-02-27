#pragma once
#include <cstdint>

namespace ps3eye {

static constexpr unsigned num_channels = 3;

struct shm_in {
    enum class mode : uint8_t { qvga, vga, };

    uint32_t settings_updated;
    uint8_t framerate, channels;
    mode resolution;
    //uint8_t sharpness, contrast, brightness hue, saturation;
    uint8_t gain, exposure, auto_gain, test_pattern;
    uint8_t do_exit;
};

struct shm_out
{
    enum class status : uint8_t { starting, running, fail, terminate, };

    uint32_t timecode;
    uint32_t settings_updated_ack;
    status status_;
    char error_string[256];
    union {
        uint8_t data_320x240[320][240][num_channels];
        uint8_t data_640x480[640][480][num_channels];
    };
};

struct shm {
    shm_out out;
    [[maybe_unused]] const char _padding[128 - sizeof(shm_out) % 128]; // NOLINT
    shm_in in;
};

} // ns ps3eye
