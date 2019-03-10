#include "camera-names.hpp"

#ifdef _WIN32
#   include <cwchar>
#   define NO_DSHOW_STRSAFE
#   include <dshow.h>
#elif defined(__unix) || defined(__linux) || defined(__APPLE__)
#   include <unistd.h>
#endif

#ifdef __linux
#   include <fcntl.h>
#   include <sys/ioctl.h>
#   include <linux/videodev2.h>
#   include <cerrno>
#   include <cstring>
#endif

#include <QDebug>

int camera_name_to_index(const QString &name)
{
    auto list = get_camera_names();
    int ret = list.indexOf(name);
    if (ret < 0)
        ret = 0;
    return ret;
}

QList<QString> get_camera_names()
{
    QList<QString> ret;
#ifdef _WIN32
    // Create the System Device Enumerator.
    HRESULT hr;
    CoInitialize(nullptr);
    ICreateDevEnum *pSysDevEnum = nullptr;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if (FAILED(hr))
    {
        qDebug() << "failed CLSID_SystemDeviceEnum" << hr;
        return ret;
    }
    // Obtain a class enumerator for the video compressor category.
    IEnumMoniker *pEnumCat = nullptr;
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

    if (hr == S_OK) {
        // Enumerate the monikers.
        IMoniker *pMoniker = nullptr;
        ULONG cFetched;
        bool hasKinect = true;
        while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
        {
            IPropertyBag *pPropBag;
            hr = pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void **)&pPropBag);
            if (SUCCEEDED(hr))	{
                // To retrieve the filter's friendly name, do the following:
                VARIANT var;
                VariantInit(&var);
                hr = pPropBag->Read(L"FriendlyName", &var, nullptr);
                if (SUCCEEDED(hr))
                {
                    // Display the name in your UI somehow.
                    QString str((QChar*)var.bstrVal, int(std::wcslen(var.bstrVal)));
                    ret.append(str);
                    if (!hasKinect && str.compare(KKinectVideoSensor)==0)
                    {
                        // We do have a Kinect V2 connected
                        hasKinect = true;
                    }
                }
                VariantClear(&var);
                pPropBag->Release();
            }
            pMoniker->Release();
        }

        if (hasKinect)
        {
            // Add Kinect V2 IR Sensor as an option then        
            ret.append(QString(KKinectIRSensor));
            // We made sure that extra camera is added at the end of our array so as not to mess up genuine camera indices...
            // ...which we relly on to create our OpenCV VideoCapture object in Camera::start.
        }

        pEnumCat->Release();
    }
    else
        qDebug() << "failed CLSID_VideoInputDeviceCategory" << hr;

    pSysDevEnum->Release();
#endif

#ifdef __linux
    for (int i = 0; i < 16; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "/dev/video%d", i);

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
            ret.append(QString{(const char*)video_cap.card});
            close(fd);
        }
    }
#endif
    return ret;
}
