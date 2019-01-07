#undef NDEBUG
#include <cassert>

#include "base-path.hpp"
#include <QCoreApplication>

const QString& application_base_path()
{
    assert(qApp && "logic error");
    static QString const& const_path = QCoreApplication::applicationDirPath();
    return const_path;
}
