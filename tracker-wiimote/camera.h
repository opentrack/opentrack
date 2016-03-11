/* Copyright (c) 2016 fred41
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <opencv2/core/core.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <QDebug>


#define EVENT_DEV_NAME "/dev/input/wii_ir"
#define IR_CACHE_SIZE 8
#define INVALID_EV_VALUE 1023
#define CAM_W 1016
#define CAM_H 760
#define CODE_MASK 7

struct CamInfo
{
    CamInfo() : res_x(CAM_W), res_y(CAM_H) {}

    int res_x;
    int res_y;
};

// ----------------------------------------------------------------------------
// WiiMote IR camera
class Camera
{
public:
    Camera() { start();}
    ~Camera() { stop();}

    void start();
    void stop();
    void set_res() {};
    bool get_info(CamInfo &ret);
    bool points_updated(std::vector<cv::Vec2f>& p);

protected:
    CamInfo cam_info;
private:
    int wii_ir_fd = -1;
    const float W = CAM_W;
    const float H = CAM_H;
	__u16 ir_cache[IR_CACHE_SIZE];
};
