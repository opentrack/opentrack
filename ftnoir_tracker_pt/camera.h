/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/core/core.hpp>
#ifndef OPENTRACK_API
#   include <boost/shared_ptr.hpp>
#else
#   include <memory>
#   include <opencv2/highgui/highgui.hpp>
#   include <opencv2/highgui/highgui_c.h>
#endif
#include <string>

// ----------------------------------------------------------------------------
void get_camera_device_names(std::vector<std::string>& device_names);


// ----------------------------------------------------------------------------
struct CamInfo
{
    CamInfo() : res_x(0), res_y(0), fps(0) {}

    int res_x;
    int res_y;
    int fps;
};

// ----------------------------------------------------------------------------
// Base class for cameras, calculates the frame rate
class Camera
{
public:
        Camera() : dt_valid(0), dt_mean(0), desired_index(0), active_index(-1), active(false) {}
        virtual ~Camera() = 0;

        // start/stop capturing
        virtual void start() = 0;
        virtual void stop() = 0;
        void restart() { stop(); start(); }

        // calls corresponding template methods and reinitializes frame rate calculation
        void set_device_index(int index);
        void set_fps(int fps);
        void set_res(int x_res, int y_res);

        // gets a frame from the camera, dt: time since last call in seconds
        bool get_frame(float dt, cv::Mat* frame);

        // WARNING: returned references are valid as long as object
        const CamInfo& get_info() const    { return cam_info; }
        const CamInfo& get_desired() const { return cam_desired; }

protected:
        // get a frame from the camera
        virtual bool _get_frame(cv::Mat* frame) = 0;

        // update the camera using cam_desired, write res and f to cam_info if successful
        virtual void _set_device_index() = 0;
        virtual void _set_fps() = 0;
        virtual void _set_res() = 0;
private:
        float dt_valid;
        float dt_mean;
protected:
        int desired_index;
        int active_index;
        bool active;
        CamInfo cam_info;
        CamInfo cam_desired;
};
inline Camera::~Camera() {}

// ----------------------------------------------------------------------------
// camera based on OpenCV's videoCapture
#ifdef OPENTRACK_API
class CVCamera : public Camera
{
public:
    CVCamera() : cap(NULL) {}
    ~CVCamera() { stop(); }

    void start() override;
    void stop() override;

protected:
    bool _get_frame(cv::Mat* frame) override;
    void _set_fps() override;
    void _set_res() override;
    void _set_device_index() override;

private:
    cv::VideoCapture* cap;
};
#else
// ----------------------------------------------------------------------------
// Camera based on the videoInput library
class VICamera : public Camera
{
public:
    VICamera();
    ~VICamera() { stop(); }

    virtual void start();
    virtual void stop();

protected:
    virtual bool _get_frame(cv::Mat* frame);
    virtual void _set_device_index();
    virtual void _set_fps();
    virtual void _set_res();

    videoInput VI;
    cv::Mat new_frame;
    unsigned char* frame_buffer;
};
#endif

enum RotationType
{
    CLOCKWISE = 0,
    ZERO = 1,
    COUNTER_CLOCKWISE = 2
};

// ----------------------------------------------------------------------------
class FrameRotation
{
public:
    RotationType rotation;

    cv::Mat rotate_frame(cv::Mat frame);
};

#endif //CAMERA_H
