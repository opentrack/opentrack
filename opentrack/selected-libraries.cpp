#include "opentrack/selected-libraries.hpp"
#include <QDebug>

SelectedLibraries::SelectedLibraries(QFrame* frame, mem<ITracker> t, dylibptr p, mem<IFilter> f) :
    pTracker(nullptr),
    pFilter(nullptr),
    pProtocol(nullptr),
    correct(false)
{
    pTracker = t;
    pProtocol = make_dylib_instance<IProtocol>(p);
    pFilter = f;

    if (!pTracker || !pProtocol)
    {
        qDebug() << "dylib load failure";
        return;
    }

    if(!pProtocol->correct())
    {
        qDebug() << "protocol load failure";
        return;
    }

    pTracker->start_tracker(frame);

    correct = true;
}
