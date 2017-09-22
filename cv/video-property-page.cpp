/* Copyright (c) 2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video-property-page.hpp"

#ifdef _WIN32

#include "compat/camera-names.hpp"
#include "opentrack-library-path.h"

#include <cstring>

#include <QRegularExpression>
#include <QProcess>
#include <QDebug>
#include <QFile>

#include <dshow.h>

#define CHECK(expr) if (FAILED(hr = (expr))) { qDebug() << QStringLiteral(#expr) << hr; goto done; }
#define CHECK2(expr) if (!(expr)) { qDebug() << QStringLiteral(#expr); goto done; }

bool video_property_page::show_from_capture(cv::VideoCapture& cap, int index)
{
    const QString name = get_camera_names().value(index, "");

    if (name == "PS3Eye Camera")
    {
        return QProcess::startDetached(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH "./amcap.exe");
    }
    else
    {
        cap.set(cv::CAP_PROP_SETTINGS, 0);
        return true;
    }
}

bool video_property_page::should_show_dialog(const QString& camera_name)
{
    using re = QRegularExpression;
    static const re regexen[] =
    {
        //re("^PS3Eye Camera$"),
        re("^A4 TECH "),
    };
    bool avail = true;
    for (const re& r : regexen)
    {
        avail &= !r.match(camera_name).hasMatch();
        if (!avail)
            break;
    }
    return avail;
}

bool video_property_page::show(int id)
{
    const QString name = get_camera_names().value(id, "");

    if (name == "PS3Eye Camera")
        return QProcess::startDetached(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH "./amcap.exe");
    else
    {
        IBaseFilter* filter = NULL;
        bool ret = false;

        CHECK2(filter = get_device(id));

        ret = SUCCEEDED(ShowFilterPropertyPages(filter));

done:
        if (filter)
            filter->Release();

        return ret;
    }
}

int video_property_page::ShowFilterPropertyPages(IBaseFilter* filter)
{
    ISpecifyPropertyPages* pProp = NULL;
    IUnknown* unk = NULL;
    CAUUID caGUID = { 0, NULL };
    FILTER_INFO FilterInfo = { {0}, NULL };
    HRESULT hr;

    CHECK(filter->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pProp));
    CHECK(pProp->GetPages(&caGUID));
    CHECK(filter->QueryFilterInfo(&FilterInfo));

    filter->QueryInterface(IID_IUnknown, (void**)&unk);

    // OleInitialize, CoCreateInstance et al. don't help with ps3 eye.

    // cl-eye uses this
    // perhaps more than IBaseFilter* -> IUnknown* needs be passed to lplpUnk
    // and the OleCreatePropertyFrame equiv
#if 0
    OCPFIPARAMS params;
    params.cbStructSize = sizeof(params);
    params.hWndOwner = GetActiveWindow();
    params.x = 0;
    params.y = 0;
    params.lpszCaption = L"camera props";
    params.cObjects = 1;
    params.lplpUnk = &unk;
    params.cPages = 1;
    //OleCreatePropertyFrameIndirect()
#endif

    OleCreatePropertyFrame(
                NULL,                   // Parent window
                0, 0,                   // Reserved
                FilterInfo.achName,     // Caption for the dialog box
                1,                      // Number of objects (just the filter)
                &unk,            // Array of object pointers.
                caGUID.cElems,          // Number of property pages
                caGUID.pElems,          // Array of property page CLSIDs
                0,                      // Locale identifier
                0, NULL                 // Reserved
                );

done:
    if (FilterInfo.pGraph)
        FilterInfo.pGraph->Release();

    if (caGUID.pElems)
        CoTaskMemFree(caGUID.pElems);

    if (pProp)
       pProp->Release();

    if (unk)
        unk->Release();

    return hr;
}

IBaseFilter* video_property_page::get_device(int id)
{
    ICreateDevEnum* pSysDevEnum = NULL;
    IEnumMoniker* pEnumCat = NULL;
    IMoniker* pMoniker = NULL;
    IBaseFilter* filter = NULL;
    HRESULT hr;

    CHECK(CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pSysDevEnum));
    CHECK(pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0));

    for (int i = 0; !filter && SUCCEEDED(pEnumCat->Next(1, &pMoniker, NULL)); pMoniker->Release(), i++)
    {
        if (i == id)
        {
            CHECK(pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&filter));
            break;
        }
    }

done:
    if (pMoniker)
        pMoniker->Release();

    if (pEnumCat)
        pEnumCat->Release();

    if (pSysDevEnum)
        pSysDevEnum->Release();

    return filter;
}

#elif defined(__linux)
#   include <QProcess>
#   include "compat/camera-names.hpp"

bool video_property_page::should_show_dialog(const QString&)
{
    return true;
}

bool video_property_page::show(int idx)
{
    const QList<QString> camera_names(get_camera_names());

    if (idx >= 0 && idx < camera_names.size())
        return QProcess::startDetached("qv4l2", QStringList() << "-d" << camera_names[idx]);
    else
        return false;
}

bool video_property_page::show_from_capture(cv::VideoCapture&, int idx)
{
    return show(idx);
}
#else
bool video_property_page::show(int) { return false; }
bool video_property_page::show_from_capture(cv::VideoCapture&, int) { return false; }
bool video_property_page::should_show_dialog(const QString& camera_name)
{
    return false;
}
#endif
