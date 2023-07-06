/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QDebug>
#include <QStringList>

#if defined _WIN32

#include <windows.h>
#include <tlhelp32.h>

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
#include <sys/types.h>
#include <sys/sysctl.h>
#include <cerrno>
#include <cstring>
#include <vector>

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
            qDebug() << "proc_listpids numproc failed" << errno;
            return ret;
        }
        vec.resize(numproc);
        int cnt = proc_listpids(PROC_ALL_PIDS, 0, &vec[0], sizeof(int) * numproc);

        if (cnt <= numproc)
        {
            std::vector<char> arglist;
            int mib[2] { CTL_KERN, KERN_ARGMAX };
            size_t sz = sizeof(int);
            int maxarg = 0;
            if (sysctl(mib, 2, &maxarg, &sz, NULL, 0) == -1)
            {
                qDebug() << "sysctl KERN_ARGMAX" << errno;
                return ret;
            }
            arglist.resize(maxarg);
            for (int i = 0; i < numproc; i++)
            {
                size_t maxarg_ = (size_t)maxarg;
                int mib[3] { CTL_KERN, KERN_PROCARGS2, vec[i] };
                if (sysctl(mib, 3, &arglist[0], &maxarg_, NULL, 0) == -1)
                {
                    //qDebug() << "sysctl KERN_PROCARGS2" << vec[i] << errno;
                    continue;
                }
                QStringList cmdline;
                for (unsigned j = sizeof(int) + strlen(&arglist[sizeof(int)]); j < maxarg_; j++)
                {
                    QString arg(&arglist[j]);
                    if (arg.size() != 0)
                    {
                        cmdline << arg;
                        j += arg.size();
                    }
                }
                if (cmdline.size() > 0)
                {
                    int idx = cmdline[0].lastIndexOf('/');
                    if (idx != -1)
                    {
                        QString tmp = cmdline[0].mid(idx+1);
                        if (cmdline.size() > 1 && (tmp == QStringLiteral("wine.bin") || tmp == QStringLiteral("wine")))
                        {
                            idx = cmdline[1].lastIndexOf('/');
                            if (idx == -1)
                                idx = cmdline[1].lastIndexOf('\\');
                            if (idx != -1)
                            {
                                ret.append(cmdline[1].mid(idx+1));
                            }
                            else
                                ret.append(cmdline[1]);
                        }
                        else
                        {
                            ret.append(tmp);
                        }
                    }
                    else
                        ret.append(cmdline[0]);
                }
            }
            return ret;
        }
    }
}

#elif defined __linux__

#include <cerrno>

#ifdef OTR_HAS_LIBPROC2
#include <libproc2/pids.h>
template<typename = void>
QStringList get_all_executable_names()
{
    QStringList ret;
    enum pids_item items[] = { PIDS_ID_PID, PIDS_CMD, PIDS_CMDLINE_V };

    enum rel_items { rel_pid, rel_cmd, rel_cmdline };
    struct pids_info *info = NULL;
    struct pids_stack *stack;
    QString tmp; tmp.reserve(64);

    procps_pids_new(&info, items, 3);

    while ((stack = procps_pids_get(info, PIDS_FETCH_TASKS_ONLY)))
    {
        char  **p_cmdline = PIDS_VAL(rel_cmdline, strv,  stack, info);

        // note, wine sets argv[0] so no parsing like in OSX case
        if (p_cmdline && p_cmdline[0] && p_cmdline[0][0] &&
            !(p_cmdline[0][0] == '-' && !p_cmdline[0][1]))
        {
            tmp = QString{p_cmdline[0]};
            const int idx = std::max(tmp.lastIndexOf('\\'), tmp.lastIndexOf('/'));
            if (idx != -1)
                tmp = tmp.mid(idx+1);
            //qDebug() << "procps" << tmp;
            ret.append(tmp);
        }
    }
    //qDebug() << "-- procps end";

    procps_pids_unref(&info);

    return ret;
}
#else
#include <proc/readproc.h>
#include <cerrno>

template<typename = void>
QStringList get_all_executable_names()
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
        // note, wine sets argv[0] so no parsing like in OSX case
        auto proc = procs[i];
        if (proc->cmdline && proc->cmdline[0])
        {
            QString tmp(proc->cmdline[0]);
            const int idx = std::max(tmp.lastIndexOf('\\'), tmp.lastIndexOf('/'));
            tmp = tmp.mid(idx == -1 ? 0 : idx+1);
            ret.append(tmp);
        }
        freeproc(procs[i]);
    }
    free(procs);
    return ret;
}
#endif

#else
template<typename = void>
static QStringList get_all_executable_names()
{
    return QStringList();
}
#endif
