#include "runtime-libraries.hpp"
#include "options/scoped.hpp"
#include <QDebug>

runtime_libraries::runtime_libraries(QFrame* frame, dylibptr t, dylibptr p, dylibptr f) :
    pTracker(nullptr),
    pFilter(nullptr),
    pProtocol(nullptr),
    correct(false)
{
    using namespace options;

    with_tracker_teardown sentinel;

    pProtocol = make_dylib_instance<IProtocol>(p);

    if (!pProtocol)
    {
        qDebug() << "protocol dylib load failure";
        goto end;
    }

    if(!pProtocol->correct())
    {
        qDebug() << "protocol load failure";
        pProtocol = nullptr;
        goto end;
    }

    pTracker = make_dylib_instance<ITracker>(t);
    pFilter = make_dylib_instance<IFilter>(f);

    if (!pTracker)
    {
        qDebug() << "tracker dylib load failure";
        goto end;
    }

    pTracker->start_tracker(frame);

    correct = true;
end:
    (void)0;
}

