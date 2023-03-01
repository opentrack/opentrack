#include "base-value.hpp"
#include <QThread>

using namespace options;

//#define OTR_TRACE_NOTIFY

const bool value_::TRACE_NOTIFY =
#ifdef OTR_TRACE_NOTIFY
    true;
#else
    [] {
    auto b = qgetenv("OTR_TRACE_NOTIFY");
    return !b.isEmpty() && b != "0";
    }();
#endif

value_::value_(bundle const& b, const QString& name) noexcept :
    b(b), self_name(name)
{
    b->on_value_created(this);
}

value_::~value_()
{
    b->on_value_destructed(this);
}

void value_::maybe_trace(const char* str) const
{
    if (TRACE_NOTIFY)
        qDebug().noquote() << str << QThread::currentThreadId() << b->name() << self_name << get_variant();
}
