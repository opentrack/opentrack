#pragma once

// this is to avoid dealing with QMetaObject for the time being -sh 20190203

#include "export.hpp"
#include "options/slider.hpp"
#include <QObject>
#include <QList>
#include <QPointF>
#include <QVariant>

namespace _qt_sig_impl {

template<typename t> struct sig;

class OTR_COMPAT_EXPORT sig_void final : public QObject
{
    Q_OBJECT

public:
    template<typename t, typename F>
    sig_void(t* datum, F&& f, Qt::ConnectionType conntype = Qt::AutoConnection) : QObject(datum)
    {
        connect(this, &sig_void::notify, datum, f, conntype);
    }
    explicit sig_void(QObject* parent = nullptr);
    void operator()() const { notify(); }

signals:
    void notify() const;
};

template<> struct sig<void> { using t = sig_void; };

#ifndef OTR_GENERATE_SIGNAL3
#   define OTR_GENERATE_SIGNAL3(x)
#endif
#define OTR_GENERATE_SIGNAL2(type)                                              \
    class OTR_COMPAT_EXPORT sig_##type final : public QObject                   \
    {                                                                           \
        Q_OBJECT                                                                \
        public:                                                                 \
            explicit sig_##type(QObject* parent = nullptr) : QObject(parent) {} \
            void operator()(const type& x) const;                               \
        Q_SIGNALS:                                                              \
            void notify(const type& x) const;                                   \
    };                                                                          \
    OTR_GENERATE_SIGNAL3(type)

#   define OTR_GENERATE_SIGNAL(type)                                            \
       OTR_GENERATE_SIGNAL2(type);                                              \
       using qlist##type = QList<type>;                                         \
       OTR_GENERATE_SIGNAL2(qlist##type);                                       \
       template<> struct sig<type> { using t = sig_##type; };                   \
       template<> struct sig<qlist##type> { using t = qlist##type; }

using slider_value = options::slider_value;

OTR_GENERATE_SIGNAL(int);
OTR_GENERATE_SIGNAL(double);
OTR_GENERATE_SIGNAL(float);
OTR_GENERATE_SIGNAL(bool);
OTR_GENERATE_SIGNAL(QString);
//OTR_GENERATE_SIGNAL(slider_value);
OTR_GENERATE_SIGNAL(QPointF);
OTR_GENERATE_SIGNAL(QVariant);

} // namespace _qt_sig_impl

#undef OTR_GENERATE_SIGNAL2
#undef OTR_GENERATE_SIGNAL

template<typename t> using qt_signal = typename _qt_sig_impl::sig<t>::t;
