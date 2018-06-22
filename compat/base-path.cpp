#include "base-path.hpp"
#include <QCoreApplication>

OTR_COMPAT_EXPORT
cc_noinline
const QString& application_base_path()
{
    static QString const& const_path = QCoreApplication::applicationDirPath();
    return const_path;
}
