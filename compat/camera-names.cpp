#include "camera-names.hpp"

#ifdef _WIN32
#   define NO_DSHOW_STRSAFE
#   include <dshow.h>
#   include <cwchar>
#elif defined(__unix) || defined(__linux) || defined(__APPLE__)
#   include <unistd.h>
#endif

#ifdef __linux
#   include <fcntl.h>
#   include <sys/ioctl.h>
#   include <linux/videodev2.h>
#   include <cerrno>
#endif

#include <QDebug>

OPENTRACK_COMPAT_EXPORT int camera_name_to_index(const QString &name)
{
    auto list = get_camera_names();
    int ret = list.indexOf(name);
    if (ret < 0)
        ret = 0;
    return ret;
}

OPENTRACK_COMPAT_EXPORT QList<QString> get_camera_names()
{
    QList<QString> ret;
#if defined(_WIN32)
    // Create the System Device Enumerator.
    HRESULT hr;
    CoInitialize(nullptr);
    ICreateDevEnum *pSysDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if (FAILED(hr))
    {
        qDebug() << "failed CLSID_SystemDeviceEnum" << hr;
        return ret;
    }
    // Obtain a class enumerator for the video compressor category.
    IEnumMoniker *pEnumCat = NULL;
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

    if (hr == S_OK) {
        // Enumerate the monikers.
        IMoniker *pMoniker = NULL;
        ULONG cFetched;
        while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
        {
            IPropertyBag *pPropBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if (SUCCEEDED(hr))	{
                // To retrieve the filter's friendly name, do the following:
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                if (SUCCEEDED(hr))
                {
                    // Display the name in your UI somehow.
                    QString str((QChar*)varName.bstrVal, int(std::wcslen(varName.bstrVal)));
                    ret.append(str);
                }
                VariantClear(&varName);

                ////// To create an instance of the filter, do the following:
                ////IBaseFilter *pFilter;
                ////hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
                ////	(void**)&pFilter);
                // Now add the filter to the graph.
                //Remember to release pFilter later.
                pPropBag->Release();
            }
            pMoniker->Release();
        }
        pEnumCat->Release();
    }
    else
        qDebug() << "failed CLSID_VideoInputDeviceCategory" << hr;

    pSysDevEnum->Release();
#endif
#ifdef __linux
    for (int i = 0; i < 16; i++) {
        char buf[128];
        sprintf(buf, "/dev/video%d", i);
        if (access(buf, F_OK) == 0)
            ret.append(buf);
        else
            continue;

        if (access(buf, R_OK | W_OK) == 0) {
            int fd = open(buf, O_RDONLY);
            if (fd == -1)
                continue;
            struct v4l2_capability video_cap;
            if(ioctl(fd, VIDIOC_QUERYCAP, &video_cap) == -1)
            {
                qDebug() << "VIDIOC_QUERYCAP" << errno;
                close(fd);
                continue;
            }
            ret[ret.size()-1] = reinterpret_cast<const char*>(video_cap.card);
            close(fd);
        }
    }
#endif
    return ret;
}
