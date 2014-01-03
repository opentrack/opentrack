/* Copyright (c) 2011-2012, Stanislaw Halik <sthalik@misaki.pl>

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

#define MEMOIZE_PRECISION 100

class FTNOIR_TRACKER_BASE_EXPORT FunctionConfig {
private:
    QMutex _mutex;
	QList<QPointF> _points;
	void reload();
    float* _data;
	int _size;
	QString _title;
    float getValueInternal(int x);
	QPointF lastValueTracked;								// The last input value requested by the Tracker, with it's output-value.
    volatile bool _tracking_active;
	int _max_Input;
	int _max_Output;
    FunctionConfig(const FunctionConfig&) = delete;
public:
    int maxInput() const { return _max_Input; }
    int maxOutput() const { return _max_Output; }
	//
	// Contructor(s) and destructor
	//
    FunctionConfig();
    FunctionConfig(QString title, int intMaxInput, int intMaxOutput);
    ~FunctionConfig();

    float getValue(float x);
	bool getLastPoint(QPointF& point);						// Get the last Point that was requested.

	//
	// Functions to manipulate the Function
	//
	void removePoint(int i);
    void removeAllPoints() {
        QMutexLocker foo(&_mutex);
        _points.clear();
        reload();
    }

	void addPoint(QPointF pt);
	void movePoint(int idx, QPointF pt);
	QList<QPointF> getPoints();
	void setMaxInput(int MaxInput) {
		_max_Input = MaxInput;
	}
	void setMaxOutput(int MaxOutput) {
		_max_Output = MaxOutput;
	}

	//
	// Functions to load/save the Function-Points to an INI-file
	//
	void saveSettings(QSettings& settings);
	void loadSettings(QSettings& settings);

	void setTrackingActive(bool blnActive);
    QString getTitle() { return _title; }
};
