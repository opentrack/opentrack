#if 0
#if 0
#include "video-property-page.hpp"
#else
#include <windows.h>
#include <dshow.h>

struct video_property_page final
{
    video_property_page() = delete;
    static bool show(int id);

private:
    static HRESULT ShowFilterPropertyPages(IBaseFilter* filter);
    static IBaseFilter* get_device(int id);
};
#endif
// above is the header, for completeness
// OleInitialize, CoCreateInstance et al. don't help with ps3 eye.

#include <cstring>

#include <QString>
#include <QDebug>

#if 0
DEFINE_GUID(CLSID_SampleGrabber,0xc1f400a0,0x3f08,0x11d3,0x9f,0x0b,0x00,0x60,0x08,0x03,0x9e,0x37);
DEFINE_GUID(IID_ISampleGrabber,0x6b652fff,0x11fe,0x4fce,0x92,0xad,0x02,0x66,0xb5,0xd7,0xc7,0x8f);

struct ISampleGrabberCB : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SampleCB(
        double SampleTime,
        IMediaSample *pSample) = 0;

    virtual HRESULT STDMETHODCALLTYPE BufferCB(
        double SampleTime,
        BYTE *pBuffer,
        LONG BufferLen) = 0;
};

struct ISampleGrabber : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetOneShot(
        BOOL OneShot) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetMediaType(
        const AM_MEDIA_TYPE *pType) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(
        AM_MEDIA_TYPE *pType) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(
        BOOL BufferThem) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(
        LONG *pBufferSize,
        LONG *pBuffer) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(
        IMediaSample **ppSample) = 0;

    virtual HRESULT STDMETHODCALLTYPE SetCallback(
        ISampleGrabberCB *pCallback,
        LONG WhichMethodToCallback) = 0;
};
#endif

#define CHECK(expr) if (FAILED(hr = (expr))) { qDebug() << QStringLiteral(#expr) << hr; goto done; }
#define CHECK2(expr) if (!(expr)) { qDebug() << QStringLiteral(#expr); goto done; }

#if 0
static IPin* GetPin(IBaseFilter *pFilter, PIN_DIRECTION dir_)
{
    IPin* ret = NULL;
    IEnumPins* pin_enum = NULL;
    PIN_DIRECTION dir;
    HRESULT hr;

    CHECK(pFilter->EnumPins(&pin_enum));

    while (SUCCEEDED(pin_enum->Next(1, &ret, NULL)))
    {
        CHECK(ret->QueryDirection(&dir));
        if (dir == dir_)
            goto done;
        ret->Release();
        ret = NULL;
    }

    ret = NULL;

done:
    if (pin_enum)
        pin_enum->Release();

    return ret;
}
#endif

bool video_property_page::show(int id)
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

HRESULT video_property_page::ShowFilterPropertyPages(IBaseFilter* filter)
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

#endif
