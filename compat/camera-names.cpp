#include "camera-names.hpp"

#include <algorithm>
#include <iterator>

#ifdef _WIN32
#   include <cwchar>
#   define NO_DSHOW_STRSAFE
#   include <dshow.h>
#   include <wrl/client.h>
#elif defined(__unix) || defined(__linux__) || defined(__APPLE__)
#   include <unistd.h>
#endif

#ifdef __APPLE__
#   include <QCameraDevice>
#   include <QMediaDevices>
#endif

#ifdef __linux__
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
    auto it = std::find_if(list.cbegin(), list.cend(), [&name](const auto& tuple) {
        const auto& [str, idx] = tuple;
        return str == name;
    });
    if (it != list.cend())
    {
        const auto& [ str, idx ] = *it;
        return idx;
    }

    return -1;
}

#ifdef _WIN32
#   include <QRegularExpression>

static QString prop_to_qstring(IPropertyBag* pPropBag, const wchar_t* name)
{
    QString ret{};
    VARIANT var;
    VariantInit(&var);
    HRESULT hr = pPropBag->Read(name, &var, nullptr);
    if (SUCCEEDED(hr))
        ret = QString{(const QChar*)var.bstrVal, int(std::wcslen(var.bstrVal))};
    VariantClear(&var);
    return ret;
}

static QString device_path_from_qstring(const QString& str)
{
    // language=RegExp prefix=R"/( suffix=)/"
    static const QRegularExpression regexp{R"/(#vid_([0-9a-f]{4})&pid_([0-9a-f]{4})&mi_([0-9a-f]{2})#([^#]+))/",
                                           QRegularExpression::CaseInsensitiveOption};
    auto match = regexp.match(str);
    if (!match.hasMatch())
        return {};
    QString id = match.captured(4);
    id.replace('&', '_');
    return id;
}

#endif

std::vector<std::tuple<QString, int>> get_camera_names()
{
    std::vector<std::tuple<QString, int>> ret;
#ifdef _WIN32
    using Microsoft::WRL::ComPtr;

    // Create the System Device Enumerator.
    HRESULT hr;
    CoInitialize(nullptr);
    ComPtr<ICreateDevEnum> pSysDevEnum;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)pSysDevEnum.GetAddressOf());
    if (hr != S_OK)
    {
        qDebug() << "failed CLSID_SystemDeviceEnum" << hr;
        return ret;
    }
    // Obtain a class enumerator for the video compressor category.
    ComPtr<IEnumMoniker> pEnumCat;
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, pEnumCat.GetAddressOf(), 0);

    if (hr == S_OK) {
        // Enumerate the monikers.
        ComPtr<IMoniker> pMoniker;
        ULONG cFetched;
        while (pEnumCat->Next(1, pMoniker.GetAddressOf(), &cFetched) == S_OK)
        {
            ComPtr<IPropertyBag> pPropBag;
            hr = pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void **)pPropBag.GetAddressOf());
            if (hr == S_OK) {
                // To retrieve the filter's friendly name, do the following:
                QString str = prop_to_qstring(pPropBag.Get(), L"FriendlyName");
                QString path = device_path_from_qstring(prop_to_qstring(pPropBag.Get(), L"DevicePath"));
                if (!path.isNull())
                    str += QStringLiteral(" [%1]").arg(path);
                ret.push_back({str, (int)ret.size()});
            }
        }
    }
#elif defined __linux__
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
            ret.push_back({ QString((const char*)video_cap.card), i});
            close(fd);
        }
    }
#elif defined __APPLE__
    for (const QCameraDevice& camera_info : QMediaDevices::videoInputs())
        ret.push_back({ camera_info.description(), ret.size() });
#endif

    return ret;
}
