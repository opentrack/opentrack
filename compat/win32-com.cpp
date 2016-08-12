#ifdef _WIN32

#include "win32-com.hpp"

#include <QString>
#include <QThread>
#include <QDebug>

bool OPENTRACK_COMPAT_EXPORT init_com_threading(com_type t)
{
    static thread_local com_type initialized = com_type(-1);

    if (initialized != com_type(-1))
    {
        if (t != initialized)
        {
            QString tp("invalid type");
            switch (t)
            {
            case com_apartment:
                tp = "apartment threaded";
                break;
            case com_multithreaded:
                tp = "multithreaded";
                break;
            }

            qDebug() << "COM for thread"
                     << QThread::currentThread() << QThread::currentThreadId()
                     << "already initialized to" << tp;

            return false;
        }

        return true;
    }

    HRESULT ret = CoInitializeEx(0, t);

    if (ret != S_OK && ret != S_FALSE)
    {
        qDebug() << "CoInitializeEx failed:" << ret << GetLastError();
        return false;
    }

    if (t == com_apartment)
    {
        ret = OleInitialize(nullptr);

        if (ret != S_OK && ret != S_FALSE)
            qDebug() << "OleInitialize() failed:" << ret << GetLastError();

        return false;
    }

    initialized = t;

    return true;
}
#endif
