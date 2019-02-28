#undef NDEBUG
#include <cassert>

#include "base-path.hpp"
#include <QCoreApplication>

const QString& application_base_path()
{
    assert(qApp && "logic error");
    static QString path = QCoreApplication::applicationDirPath();
    return path;
}
