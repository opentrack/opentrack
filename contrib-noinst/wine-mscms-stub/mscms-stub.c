#include <windows.h>

__declspec(dllexport) void* __stdcall WcsCreateIccProfile(void* a, unsigned long b)
{
    (void)a; (void)b;
    return NULL;
}

__declspec(dllexport) BOOL __stdcall CloseColorProfile(void* a)
{
    (void)a;
    return FALSE;
}

__declspec(dllexport) BOOL __stdcall GetColorProfileFromHandle(void* a, unsigned char* b, unsigned long* c)
{
    (void)a; (void)b; (void)c;
    return FALSE;
}

__declspec(dllexport) void* __stdcall OpenColorProfileW(void* a, unsigned long b, unsigned long c, unsigned long d)
{
    (void)a; (void)b; (void)c; (void)d;
    return NULL;
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, void* reserved)
{
    (void)h; (void)reason; (void)reserved;
    return TRUE;
}
