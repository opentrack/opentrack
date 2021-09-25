#pragma once

#include <algorithm>
#include <QWidget>

static inline double screen_dpi(const QPaintDevice* widget)
{
    const auto& x = *widget;
#ifdef _WIN32
    return std::max(x.devicePixelRatioF(), 1.);
#else
    return std::max(std::max(x.logicalDpiX()/(double)x.physicalDpiX(), x.devicePixelRatioF()), 1.);
#endif
}

template<typename self>
struct screen_dpi_mixin
{
protected:
    double screen_dpi() const
    {
        return ::screen_dpi(static_cast<const self*>(this));
    }
};
