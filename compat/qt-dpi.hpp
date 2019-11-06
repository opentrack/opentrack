#pragma once

#include <algorithm>
#include <QWidget>

template<typename self>
struct screen_dpi_mixin
{
protected:
    double screen_dpi() const
    {
        const auto& x = *static_cast<const self*>(this);
#ifdef _WIN32
        return std::max(x.devicePixelRatioF(), 1.);
#else
        return std::max(std::max(x.logicalDpiX()/(double)x.physicalDpiX(), x.devicePixelRatioF()), 1.);
#endif
    }
};
