#pragma once

// this is to avoid dealing with QMetaObject for the time being -sh 20190203

#include "export.hpp"
#include <QObject>

namespace qt_sig {

class OTR_COMPAT_EXPORT nullary : public QObject
{
    Q_OBJECT

public:
    template<typename t, typename F>
    nullary(t* datum, F&& f, Qt::ConnectionType conntype = Qt::AutoConnection) : QObject(datum)
    {
        connect(this, &nullary::notify, datum, f, conntype);
    }

    nullary(QObject* parent = nullptr);
    ~nullary() override;

    void operator()() const;

signals:
    void notify() const;
};

} // ns qt_sig
