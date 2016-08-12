#include "selected-libraries.hpp"
#include <QDebug>

SelectedLibraries::SelectedLibraries(QFrame* frame, dylibptr t, dylibptr p, dylibptr f) :
    pTracker(nullptr),
    pFilter(nullptr),
    pProtocol(nullptr),
    correct(false)
{
    pProtocol = make_dylib_instance<IProtocol>(p);

    if (!pProtocol)
    {
        qDebug() << "protocol dylib load failure";
        return;
    }

    if(!pProtocol->correct())
    {
        qDebug() << "protocol load failure";
        pProtocol = nullptr;
        return;
    }

    pTracker = make_dylib_instance<ITracker>(t);
    pFilter = make_dylib_instance<IFilter>(f);

    if (!pTracker)
    {
        qDebug() << "tracker dylib load failure";
        return;
    }

    pTracker->start_tracker(frame);

    correct = true;
}
