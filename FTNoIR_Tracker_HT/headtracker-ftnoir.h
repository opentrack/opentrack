#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "ht-api.h"

#define HT_SHM_NAME "ftnoir-tracker-ht-shm"
#define HT_MUTEX_NAME "ftnoir-tracker-ht-mutex"

#define HT_MAX_VIDEO_WIDTH 2048
#define HT_MAX_VIDEO_HEIGHT 1536
#define HT_MAX_VIDEO_CHANNELS 3

typedef struct {
    int width, height, channels;
    unsigned char frame[HT_MAX_VIDEO_WIDTH * HT_MAX_VIDEO_HEIGHT * HT_MAX_VIDEO_CHANNELS];
} ht_video_t;

typedef struct {
    ht_video_t frame;
    ht_config_t config;
    ht_result_t result;
    volatile int timer;
    volatile bool pause, terminate, running;
} ht_shm_t;
