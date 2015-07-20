/* Copyright (c) 2014 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <QTimer>
#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QPaintEvent>

class HTVideoWidget : public QWidget
{
    Q_OBJECT
public:
    HTVideoWidget(QWidget *parent) : QWidget(parent), fb(), width(0), height(0), fresh(false) {
        connect(&timer, SIGNAL(timeout()), this, SLOT(update_and_repaint()));
        timer.start(60);
    }
    void update_image(unsigned char* frame, int width, int height);
protected slots:
    void paintEvent( QPaintEvent* e ) {
        QMutexLocker foo(&mtx);
        QPainter painter(this);
        painter.drawImage(e->rect(), texture);
    }
    void update_and_repaint();
private:
    QMutex mtx;
    QImage texture;
    QTimer timer;
    unsigned char fb[2048*2048*3];
    int width,height;
    bool fresh;
};
