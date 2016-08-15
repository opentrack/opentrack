#ifdef _WIN32

#include "win32-com.hpp"

#include "compat/util.hpp"

#include <QString>
#include <QThread>
#include <QDebug>

bool OPENTRACK_COMPAT_EXPORT init_com_threading(com_type t)
{
    HRESULT ret = CoInitializeEx(0, t);

    if (ret != S_OK && ret != S_FALSE)
    {
        qDebug() << "CoInitializeEx failed" << (progn (
                                                     switch (ret)
                                                     {
                                                         case RPC_E_CHANGED_MODE:
                                                            return QStringLiteral("COM threading mode already set");
                                                         default:
                                                            return QStringLiteral("Unknown error ") + QString::number(long(ret));
                                                     }
                                                     ));
        return false;
    }

    return true;
}
#endif
