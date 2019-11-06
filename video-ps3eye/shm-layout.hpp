#pragma once
#include <cstdint>

namespace ps3eye {

struct shm_out {
    enum class mode : uint8_t { qvga, vga, };
    enum class status : uint8_t { starting, running, fail, terminate, };

    uint8_t settings_updated;
    uint16_t framerate;
    mode resolution;
    status status_;
    uint8_t sharpness, contrast, brightness, hue, saturation;
    uint8_t gain, exposure, auto_gain, awb, test_pattern;
};

struct shm_in {
    uint8_t settings_updated_ack;
    uint8_t timecode;
    union {
        uint8_t data_320x240[320][240][3];
        uint8_t data_640x480[640][480][3];
    };
};

struct shm {
    static constexpr unsigned _cacheline_len = 64;
    static constexpr unsigned _padding_len =
        (_cacheline_len - (sizeof(shm_in) & (_cacheline_len - 1))) & (_cacheline_len - 1);

    using resolution = shm_out::mode;
    using status     = shm_out::status;

    shm_out out;
    const char* _padding[_padding_len];
    shm_in in;
};

} // ns ps3eye
