/* Copyright (c) 2016 fred41
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera.h"

bool Camera::get_info(CamInfo& ret)
{
    ret = cam_info;
    return true;
}

void Camera::start()
{

    stop();

	const char *device = EVENT_DEV_NAME;

	if ((wii_ir_fd = open(device, O_RDONLY | O_NONBLOCK | O_CLOEXEC)) < 0) {
		perror("ev_wii_ir");
		if (errno == EACCES && getuid() != 0)
			fprintf(stderr, "You do not have access to %s.\n", device);

    } else {
    
        // grab the device for our process
        int rc = ioctl(wii_ir_fd, EVIOCGRAB, (void*)1);
    }

    
    for (int i=0; i<IR_CACHE_SIZE; i++) {
        ir_cache[i] = INVALID_EV_VALUE;
    }
}

void Camera::stop()
{
	if (wii_ir_fd != -1) {
        int rc = ioctl(wii_ir_fd, EVIOCGRAB, (void*)0);
        close(wii_ir_fd);
    }
}

bool Camera::points_updated( std::vector<cv::Vec2f>& points)
{
    struct input_event ev[16];
	fd_set rdfs;
    struct timeval tv;

    if (wii_ir_fd == -1) 
        return false;

	FD_ZERO(&rdfs);
	FD_SET(wii_ir_fd, &rdfs);
    tv.tv_sec = 0;
    tv.tv_usec = 1;
	
    int ret = select(wii_ir_fd + 1, &rdfs, NULL, NULL, &tv);
	
    if (ret == -1 || ret == 0)
        return false;

    ret = read(wii_ir_fd, &ev[0], sizeof(ev));

    if (ret < sizeof(input_event))
        return false;

    for (int i = 0; i < ret / sizeof(struct input_event); i++) {
        if (ev[i].type == EV_ABS) {
            ir_cache[ev[i].code & CODE_MASK] = ev[i].value;
        }
    }

    points.clear();
    cv::Vec2f p;
	for (int i = 0; i < IR_CACHE_SIZE/2; i++) {
        unsigned x = ir_cache[i*2];
        unsigned y = ir_cache[i*2+1];
        if (x < CAM_W && y < CAM_H) {
            p[0] = (x - W * .5f) / W;
            p[1] = -(y - H * .5f) / W; 
            points.push_back(p);
        }
    }
/*
    qDebug() << "ps: " << points.size();
    for (int i = 0; i < points.size(); i++)
        qDebug() << points[i][0] << ":" << points[i][1];

    for (int i = 0; i < IR_CACHE_SIZE/2; i++)
        qDebug() << ir_cache[i*2] << ":" << ir_cache[i*2 + 1];
*/
	return true;
}
