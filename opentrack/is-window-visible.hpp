#ifdef _WIN32

#include <windows.h>
#include <QWidget>
#include <QRect>
#include <QDebug>

template<typename=void>
bool is_window_visible(const QWidget* widget)
{
    const QWidget* window = widget->window();

    if (!window)
        return true;

    const QPoint p = widget->mapToGlobal(widget->pos());
    const QSize s = widget->size();
    POINT pt = { p.x() + s.width()/2, p.y() + s.height()/2 };

    return WindowFromPoint(pt) == (HWND) widget->winId();
}

#else
template<typename=void>
bool is_window_visible(const QWidget* widget) { return true; }
#endif
