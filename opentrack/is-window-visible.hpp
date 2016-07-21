#ifdef _WIN32

#include <windows.h>
#include <QWidget>
#include <QRect>
#include <QDebug>
#include <QPoint>
#include <initializer_list>

template<typename=void>
bool is_window_visible(const QWidget* widget)
{
    const QWidget* window = widget->window();

    if (!window)
        return true;

    const QPoint p = widget->mapToGlobal(QPoint(0, 0));
    const QSize s = widget->size();

    const std::initializer_list<POINT> points =
    {
        { p.x(), p.y() },
        { p.x() + s.width(), p.y() },
        { p.x() + s.width(), p.y() + s.height() },
        { p.x(), p.y() + s.height() },
        { p.x() + s.width()/2, p.y() + s.height()/2 },
    };

    for (const POINT& pt : points)
        if (WindowFromPoint(pt) == (HWND) widget->winId())
            return true;
    return false;
}

#else
template<typename=void>
bool is_window_visible(const QWidget* widget) { return true; }
#endif
