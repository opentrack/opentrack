#include "check-visible.hpp"

#if defined _WIN32

#include "timer.hpp"

#include <QMutexLocker>

#include <windows.h>

constexpr int visible_timeout = 5000;
constexpr int invisible_timeout = 250;

static Timer timer;
static QMutex mtx;
static bool visible = true;

never_inline OTR_COMPAT_EXPORT
void set_is_visible(const QWidget& w, bool force)
{
    QMutexLocker l(&mtx);

    if (!force && timer.elapsed_ms() < (visible ? visible_timeout : invisible_timeout))
        return;

    timer.start();

    const HWND id = (HWND) w.winId();
    const QPoint pt = w.mapToGlobal({ 0, 0 });

    const int W = w.width(), H = w.height();

    const QPoint points[] =
    {
        pt,
        pt + QPoint(W - 1, 0),
        pt + QPoint(0, H - 1),
        pt + QPoint(W - 1, H - 1),
        pt + QPoint(W / 2, H / 2),
    };

    for (const QPoint& pt : points)
    {
        visible = WindowFromPoint({ pt.x(), pt.y() }) == id;
        if (visible)
            break;
    }
}

never_inline OTR_COMPAT_EXPORT
bool check_is_visible()
{
    QMutexLocker l(&mtx);

    return visible;
}

#else

never_inline OTR_COMPAT_EXPORT
void set_is_visible(const QWidget&, bool)
{
}

never_inline  OTR_COMPAT_EXPORT
bool check_is_visible()
{
    return true;
}

#endif
