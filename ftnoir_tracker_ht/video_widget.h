/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTime>
#include <memory>
#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
#include <QLabel>
#include <QPainter>

// ----------------------------------------------------------------------------
class VideoWidget : public QLabel
{
	Q_OBJECT

public:
    VideoWidget(QWidget *parent) : QLabel(parent), mtx() {
	}
    void update_image(unsigned char* frame, int width, int height);
protected slots:
    void paintEvent( QPaintEvent* e ) {
        setPixmap(pixmap);
        QLabel::paintEvent(e);
    }
private:
    QMutex mtx;
    QPixmap pixmap;
};

#endif // VIDEOWIDGET_H
