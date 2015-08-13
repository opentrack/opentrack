#pragma once

#include <QDebug>
#include <QStringList>

#if defined _WIN32

#include <windows.h>
#include <TlHelp32.h>

template<typename = void>
static QStringList get_all_executable_names()
{
    QStringList ret;
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (h == INVALID_HANDLE_VALUE)
        return ret;

    PROCESSENTRY32 e;
    e.dwSize = sizeof(e);

    if (Process32First(h, &e) != TRUE)
    {
        CloseHandle(h);
        return ret;
    }

    do {
        ret.append(e.szExeFile);
    } while (Process32Next(h, &e) == TRUE);

    CloseHandle(h);

    return ret;
}
#elif defined __APPLE__
#include <libproc.h>
#include <sys/param.h>
#include <cerrno>
#include <vector>

// link to libproc
template<typename = void>
static QStringList get_all_executable_names()
{
    QStringList ret;
    std::vector<int> vec;

    while (true)
    {
        int numproc = proc_listpids(PROC_ALL_PIDS, 0, nullptr, 0);
        if (numproc == -1)
        {
            qDebug() << "numproc failed" << errno;
            break;
        }
        vec.resize(numproc);
        int cnt = proc_listpids(PROC_ALL_PIDS, 0, &vec[0], sizeof(int) * numproc);
        if (cnt <= numproc)
        {
            char name[2 * 2 * MAXCOMLEN + 1];
            for (int i = 0; i < cnt; i++)
            {
                int ret = proc_name(vec[i], name, sizeof(name)-1);
                if (ret <= 0)
                    continue;
                name[ret] = '\0';
                ret.append(name);
            }
            return ret;
        }
    }
}

#elif defined __linux

// link to procps
#include <proc/readproc.h>
#include <cerrno>
template<typename = void>
static QStringList get_all_executable_names()
{
    QStringList ret;
    proc_t** procs = readproctab(PROC_FILLCOM);
    if (procs == nullptr)
    {
        qDebug() << "readproctab" << errno;
        return ret;
    }
    for (int i = 0; procs[i]; i++)
    {
        auto& proc = *procs[i];
        ret.append(proc.cmd);
    }
    freeproctab(procs);
    return ret;
}

#else
template<typename = void>
static QStringList get_all_executable_names()
{
    return QStringList();
}
#endif
