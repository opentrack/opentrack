#include "check-visible.hpp"

#include <QWidget>
#include <QDebug>

#include "spinlock.hpp"

static std::atomic_flag lock = ATOMIC_FLAG_INIT;
static bool visible = true;

#if defined _WIN32

#include "timer.hpp"
#include "macros.hpp"

static Timer timer;

constexpr int visible_timeout = 1000;
constexpr int invisible_timeout = 250;

#include <windows.h>

void set_is_visible(const QWidget& w, bool force)
{
    spinlock_guard l(lock);

    if (w.isHidden() || w.windowState() & Qt::WindowMinimized)
    {
        visible = false;
        return;
    }

    HWND hwnd = (HWND)w.winId();

    if (!force && timer.elapsed_ms() < (visible ? visible_timeout : invisible_timeout))
        return;

    timer.start();

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

#else

void set_is_visible(const QWidget& w, bool)
{
    spinlock_guard l(lock);
    visible = !(w.isHidden() || w.windowState() & Qt::WindowMinimized);
}

#endif

bool check_is_visible()
{
    spinlock_guard l(lock);
    return visible;
}

void force_is_visible(bool value)
{
    spinlock_guard l(lock);
    visible = value;
}
