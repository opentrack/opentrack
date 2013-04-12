/* Copyright (c) 2012, 2013 Stanisław Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <QCoreApplication>
#include <QPointF>
#include <QList>
#include "functionconfig.h"
#include <QtAlgorithms>
#include <QtAlgorithms>
#include <QSettings>
#include <math.h>
#include <QPixmap>
#include <QDebug>

//
// Constructor with List of Points in argument.
//
FunctionConfig::FunctionConfig(QString title, int intMaxInput, int intMaxOutput) :
    _mutex(QMutex::Recursive)
{
	_title = title;
    _points = QList<QPointF>();
	_data = 0;
	_size = 0;
	lastValueTracked = QPointF(0,0);
	_tracking_active = false;
	_max_Input = intMaxInput;					// Added WVR 20120805
	_max_Output = intMaxOutput;
    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );
    loadSettings(iniFile);
	reload();
}

//
// Calculate the value of the function, given the input 'x'.
// Used to draw the curve and, most importantly, to translate input to output.
// The return-value is also stored internally, so the Widget can show the current value, when the Tracker is running.
//
float FunctionConfig::getValue(float x) {
    int x2 = (int) (std::min<float>(std::max<float>(x, -360), 360) * MEMOIZE_PRECISION);
    _mutex.lock();
    float ret = getValueInternal(x2);
	lastValueTracked.setX(x);
	lastValueTracked.setY(ret);
    _mutex.unlock();
	return ret;
}

//
// The return-value is also stored internally, so the Widget can show the current value, when the Tracker is running.
//
bool FunctionConfig::getLastPoint(QPointF& point ) {

    _mutex.lock();
	point = lastValueTracked;
    _mutex.unlock();
	
	return _tracking_active;
}

float FunctionConfig::getValueInternal(int x) {
    float sign = x < 0 ? -1 : 1;
	x = x < 0 ? -x : x;
    float ret;
	if (!_data)
		ret = 0;
	else if (_size == 0)
		ret = 0;
	else if (x < 0)
		ret = 0;
	else if (x < _size && x >= 0)
		ret = _data[x];
	else
		ret = _data[_size - 1];
	return ret * sign;
}

static __inline QPointF ensureInBounds(QList<QPointF> points, int i) {
	int siz = points.size();
	if (siz == 0 || i < 0)
		return QPointF(0, 0);
	if (siz > i)
		return points[i];
	return points[siz - 1];
}

static bool sortFn(const QPointF& one, const QPointF& two) {
	return one.x() < two.x();
}

void FunctionConfig::reload() {
	_size = 0;

	if (_points.size())
		qStableSort(_points.begin(), _points.end(), sortFn);

	if (_data)
        delete[] _data;
	_data = NULL;
	if (_points.size()) {
        _data = new float[_size = MEMOIZE_PRECISION * _points[_points.size() - 1].x()];

        for (int i = 0; i < _size; i++)
                _data[i] = -1e6;

		for (int k = 0; k < _points[0].x() * MEMOIZE_PRECISION; k++) {
            if (k < _size)
                _data[k] = _points[0].y() * k / (_points[0].x() * MEMOIZE_PRECISION);
		}

       for (int i = 0; i < _points.size(); i++) {
            QPointF p0 = ensureInBounds(_points, i - 1);
            QPointF p1 = ensureInBounds(_points, i);
            QPointF p2 = ensureInBounds(_points, i + 1);
            QPointF p3 = ensureInBounds(_points, i + 2);

            int end = p2.x() * MEMOIZE_PRECISION;
            int start = p1.x() * MEMOIZE_PRECISION;

            for (int j = start; j < end && j < _size; j++) {
                float t = (j - start) / (float) (end - start);
                float t2 = t*t;
                float t3 = t*t*t;

                int x = .5 * ((2 * p1.x()) +
                              (-p0.x() + p2.x()) * t +
                              (2 * p0.x() - 5 * p1.x() + 4 * p2.x() - p3.x()) * t2 +
                              (-p0.x() + 3 * p1.x() - 3 * p2.x() + p3.x()) * t3)
                        * MEMOIZE_PRECISION;

                float y = .5 * ((2 * p1.y()) +
                                 (-p0.y() + p2.y()) * t +
                                 (2 * p0.y() - 5 * p1.y() + 4 * p2.y() - p3.y()) * t2 +
                                 (-p0.y() + 3 * p1.y() - 3 * p2.y() + p3.y()) * t3);

                if (x < _size)
                    _data[x] = y;
            }
		}

       float last = 0;

       for (int i = 0; i < _size; i++)
       {
           if (_data[i] == -1e6)
               _data[i] = last;
           last = _data[i];
       }
	}
}

FunctionConfig::~FunctionConfig() {
	if (_data)
        delete[] _data;
}

//
// Remove a Point from the Function.
// Used by the Widget.
//
void FunctionConfig::removePoint(int i) {
    _mutex.lock();
	_points.removeAt(i);
	reload();
    _mutex.unlock();
}

//
// Add a Point to the Function.
// Used by the Widget and by loadSettings.
//
void FunctionConfig::addPoint(QPointF pt) {
    _mutex.lock();
	_points.append(pt);
	reload();
    _mutex.unlock();
}

//
// Move a Function Point.
// Used by the Widget.
//
void FunctionConfig::movePoint(int idx, QPointF pt) {
    _mutex.lock();
	_points[idx] = pt;
	reload();
    _mutex.unlock();
}

//
// Return the List of Points.
// Used by the Widget.
//
QList<QPointF> FunctionConfig::getPoints() {
	QList<QPointF> ret;
    _mutex.lock();
	for (int i = 0; i < _points.size(); i++) {
		ret.append(_points[i]);
	}
    _mutex.unlock();
	return ret;
}

//
// Load the Points for the Function from the INI-file designated by settings.
// Settings for a specific Curve are loaded from their own Group in the INI-file.
//
void FunctionConfig::loadSettings(QSettings& settings) {
    QPointF newPoint;

	QList<QPointF> points;
	settings.beginGroup(QString("Curves-%1").arg(_title));
	
    int max = settings.value("point-count", 0).toInt();

    qDebug() << _title << "count" << max;

	for (int i = 0; i < max; i++) {
        newPoint = QPointF(settings.value(QString("point-%1-x").arg(i), (i + 1) * _max_Input/2).toFloat(),
                           settings.value(QString("point-%1-y").arg(i), (i + 1) * _max_Output/2).toFloat());
        //
		// Make sure the new Point fits in the Function Range.
		// Maybe this can be improved?
		//
		if (newPoint.x() > _max_Input) {
			newPoint.setX(_max_Input);
		}
		if (newPoint.y() > _max_Output) {
			newPoint.setY(_max_Output);
		}
		points.append(newPoint);
	}
    settings.endGroup();
    _mutex.lock();
	_points = points;
	reload();
    _mutex.unlock();
}

//
// Save the Points for the Function to the INI-file designated by settings.
// Settings for a specific Curve are saved in their own Group in the INI-file.
// The number of Points is also saved, to make loading more convenient.
//
void FunctionConfig::saveSettings(QSettings& settings) {
    _mutex.lock();
	settings.beginGroup(QString("Curves-%1").arg(_title));
	int max = _points.size();
	settings.setValue("point-count", max);
	for (int i = 0; i < max; i++) {
		settings.setValue(QString("point-%1-x").arg(i), _points[i].x());
		settings.setValue(QString("point-%1-y").arg(i), _points[i].y());
    }

    for (int i = max; true; i++)
    {
        QString x = QString("point-%1-x").arg(i);
        if (!settings.contains(x))
            break;
        settings.remove(x);
        settings.remove(QString("point-%1-y").arg(i));
    }
	settings.endGroup();
    _mutex.unlock();
}
