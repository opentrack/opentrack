#ifdef _WIN32

#include "win32-com.hpp"

#include "compat/util.hpp"

#include <QString>
#include <QCoreApplication>
#include <QApplication>
#include <QThread>
#include <QDebug>

bool OPENTRACK_COMPAT_EXPORT init_com_threading(com_type t_)
{
    const com_type t = progn(
                           if (t_ != com_invalid)
                               return t_;
                           if (QCoreApplication::instance() == nullptr)
                               return com_apartment;
                           if (qApp->thread() == QThread::currentThread())
                               return com_apartment;
                           else
                               return com_multithreaded;
                       );

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
