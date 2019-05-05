#pragma once

#include <cstdint>

extern const uint8_t sensor_start_qvga[8][2];
extern const uint8_t bridge_start_qvga[9][2];
extern const uint8_t sensor_start_vga[8][2];
extern const uint8_t bridge_start_vga[9][2];
extern const uint8_t ov772x_reg_initdata[35][2];
extern const uint8_t ov534_reg_initdata[47][2];

extern const struct rate_s rate_0[14];
extern const struct rate_s rate_1[22];

struct rate_s {
    uint16_t fps;
    uint8_t r11;
    uint8_t r0d;
    uint8_t re5;
};
