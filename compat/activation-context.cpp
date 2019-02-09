#ifdef _WIN32

#include "activation-context.hpp"
#include "compat/library-path.hpp"

#include <QString>
#include <QFile>
#include <QDebug>

#include <windows.h>

static_assert(sizeof(std::uintptr_t) == sizeof(ULONG_PTR));

activation_context::activation_context(const QString& module_name, int resid)
{
    static const QString prefix = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH + OPENTRACK_LIBRARY_PREFIX;
    QString path = prefix + module_name;
    QByteArray name = QFile::encodeName(path);

    ACTCTXA actx = {};
    actx.cbSize = sizeof(actx);
    actx.lpResourceName = MAKEINTRESOURCEA(resid);
    actx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
    actx.lpSource = name.constData();

    handle = CreateActCtxA(&actx);

    if (handle != INVALID_HANDLE_VALUE)
    {
        if (!ActivateActCtx(handle, (ULONG_PTR*)&cookie))
        {
            qDebug() << "win32: can't set activation context" << GetLastError();
            ReleaseActCtx(handle);
            handle = INVALID_HANDLE_VALUE;
        }
        else
            ok = true;
    } else {
        qDebug() << "win32: can't create activation context" << GetLastError();
    }
}

activation_context::~activation_context()
{
    if (handle != INVALID_HANDLE_VALUE)
    {
        DeactivateActCtx(0, cookie);
        ReleaseActCtx(handle);
    }
}
#endif
