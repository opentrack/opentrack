/* Copyright (c) 2011-2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QList>
#include <QPointF>
#include <QString>
#include <QSettings>
#include <QMutex>
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include <vector>

#define MEMOIZE_PRECISION 100

class MyMutex {
private:
    QMutex inner;
    
public:
    QMutex* operator->() { return &inner; }
    QMutex* operator->() const { return &const_cast<MyMutex*>(this)->inner; }
    
    MyMutex operator=(const MyMutex& datum)
    {
        auto mode =
                datum->isRecursive()
                ? QMutex::Recursive
                : QMutex::NonRecursive;
        
        return MyMutex(mode);
    }
    
    MyMutex(const MyMutex& datum)
    {
        *this = datum;
    }
    
    MyMutex(QMutex::RecursionMode mode = QMutex::NonRecursive) :
        inner(mode)
    {
    }
    
    QMutex* operator&()
    {
        return &inner;
    }
};

class FTNOIR_TRACKER_BASE_EXPORT FunctionConfig {
private:
    void reload();
    float getValueInternal(int x);
    
    MyMutex _mutex;
	QList<QPointF> input;
    std::vector<float> data;
	QPointF last_input_value;
    volatile bool activep;
	int max_x;
	int max_y;
public:
    int maxInput() const { return max_x; }
    int maxOutput() const { return max_y; }
    FunctionConfig();
    FunctionConfig(int maxx, int maxy)
    {
        setMaxInput(maxx);
        setMaxOutput(maxy);
    }

    float getValue(float x);
	bool getLastPoint(QPointF& point);
	void removePoint(int i);
    void removeAllPoints() {
        QMutexLocker foo(&_mutex);
        input.clear();
        reload();
    }

	void addPoint(QPointF pt);
	void movePoint(int idx, QPointF pt);
	QList<QPointF> getPoints();
	void setMaxInput(int MaxInput) {
		max_x = MaxInput;
	}
	void setMaxOutput(int MaxOutput) {
		max_y = MaxOutput;
	}

	void saveSettings(QSettings& settings, const QString& title);
	void loadSettings(QSettings& settings, const QString& title);

	void setTrackingActive(bool blnActive);
};
