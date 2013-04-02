/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QGLWidget>
#include <QTime>
#include <QFrame>
#include <QImage>
#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
// ----------------------------------------------------------------------------
class VideoWidget : public QGLWidget
{
	Q_OBJECT

public:
    VideoWidget(QWidget *parent) : QGLWidget(parent) {
#if !defined(_WIN32)
        setAttribute(Qt::WA_NativeWindow, true);
#endif
	}

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

    void updateImage(unsigned char* frame, int width, int height);
    void update();
private:
    void resize_frame(QImage& qframe);
    QImage resized_qframe;
    QMutex mtx;
};

#endif // VIDEOWIDGET_H
