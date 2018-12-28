#include "check-visible.hpp"

#if defined _WIN32

#include "timer.hpp"
#include "math.hpp"

#include <QMutex>
#include <QDebug>

#include <windows.h>

constexpr int visible_timeout = 1000;
constexpr int invisible_timeout = 250;

static Timer timer;
static QMutex mtx;
static bool visible = true;

void set_is_visible(const QWidget& w, bool force)
{
    QMutexLocker l(&mtx);

    if (!force && timer.elapsed_ms() < (visible ? visible_timeout : invisible_timeout))
        return;

    timer.start();
    HWND hwnd = (HWND)w.winId();

    if (RECT r; GetWindowRect(hwnd, &r))
    {
        const int x = r.left+1, y = r.top+1;
        const int w = r.right - x - 1, h = r.bottom - y - 1;

        const POINT xs[] {
            { x + w, y },
            { x, y + h },
            { x + w, h + y },
            { x, y },
            { x + w/2, y + h/2 },
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

bool check_is_visible()
{
    QMutexLocker l(&mtx);

    return visible;
}

#else

void set_is_visible(const QWidget&, bool)
{
}

bool check_is_visible()
{
    return true;
}

#endif
