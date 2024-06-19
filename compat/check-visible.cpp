#include "check-visible.hpp"

#include <QMutex>
#include <QWidget>
#include <QDebug>

static QMutex lock;
static bool visible = true;

#if defined _WIN32

#include "timer.hpp"
#include "macros.h"

static Timer timer;

constexpr int visible_timeout = 1000;
constexpr int invisible_timeout = 250;

#include <windows.h>

void set_is_visible(const QWidget& w, bool force)
{
    QMutexLocker l(&lock);

    if (w.isHidden() || w.windowState() & Qt::WindowMinimized)
    {
        visible = false;
        return;
    }

    {
        int ndisplays = GetSystemMetrics(SM_CMONITORS);
        if (ndisplays > 1)
        {
            visible = true;
            return;
        }
    }

    HWND hwnd = (HWND)w.winId();

    if (!force && timer.elapsed_ms() < (visible ? visible_timeout : invisible_timeout))
        return;

    timer.start();

    if (RECT r; GetWindowRect(hwnd, &r))
    {
        const int x = r.left, y = r.top;
        const int w = r.right - x, h = r.bottom - y;

        const POINT xs[] {
            { x + w - 1, y + 1     },
            { x + 1,     y + h - 1 },
            { x + w - 1, y + h - 1 },
            { x + 1,     y + 1     },
            { x + w/2,   y + h/2   },
        };

        visible = false;

        for (const POINT& pt : xs)
            if (WindowFromPoint(pt) == hwnd)
            {

                visible = true;
                break;
            }
    }
    else
    {
        eval_once(qDebug() << "check-visible: GetWindowRect failed");
        visible = true;
    }
}

#else

void set_is_visible(const QWidget& w, bool)
{
    QMutexLocker l(&lock);
    visible = !(w.isHidden() || w.windowState() & Qt::WindowMinimized);
}

#endif

bool check_is_visible()
{
    QMutexLocker l(&lock);
    return visible;
}

void force_is_visible(bool value)
{
    QMutexLocker l(&lock);
    visible = value;
}
