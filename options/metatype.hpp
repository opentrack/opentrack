#pragma once

#include <QList>
#include <QString>
#include <QPointF>
#include <QDebug>

#include "slider.hpp"
#include "defs.hpp"
#include <QMetaType>

#ifndef OPENTRACK_METATYPE_
#   define OPENTRACK_METATYPE(x) Q_DECLARE_METATYPE(x)
#else
#   define OPENTRACK_METATYPE(x) Q_DECLARE_METATYPE(x) OPENTRACK_METATYPE_(x)
#endif

#if !defined __clang_analyzer__ && !defined Q_CREATOR_RUN
OPENTRACK_METATYPE(QList<double>);
OPENTRACK_METATYPE(QList<float>);
OPENTRACK_METATYPE(QList<int>);
OPENTRACK_METATYPE(QList<bool>);
OPENTRACK_METATYPE(QList<QString>);
OPENTRACK_METATYPE(QList<QPointF>);
OPENTRACK_METATYPE(::options::slider_value);
#endif
