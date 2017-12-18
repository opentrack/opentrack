#define _USE_MATH_DEFINES

#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include <windows.h>

#define FREETRACK_MUTEX "FT_Mutext"
#define FT_MM_DATA "FT_SharedMem"

#define UNUSED(var) (void)var

typedef struct TFreeTrackData
{
    int DataID;
    int CamWidth;
    int CamHeight;
    float Yaw;
    float Pitch;
    float Roll;
    float X;
    float Y;
    float Z;
    float RawYaw;
    float RawPitch;
    float RawRoll;
    float RawX;
    float RawY;
    float RawZ;
    float X1;
    float Y1;
    float X2;
    float Y2;
    float X3;
    float Y3;
    float X4;
    float Y4;
} TFreeTrackData;

typedef struct FTMemMap
{
    TFreeTrackData data;
    __int32 GameId;
    unsigned char table[8];
    __int32 GameId2;
} FTMemMap;

static bool bEncryptionChecked = false;
static double r = 0, p = 0, y = 0, tx = 0, ty = 0, tz = 0;

#define NP_DECLSPEC __declspec(dllexport)
#define NP_EXPORT(t) t NP_DECLSPEC __stdcall
#define NP_AXIS_MAX 16384

static bool FTCreateMapping(void);
static void FTDestroyMapping(void);
static __inline double clamp(double x, double xmin, double xmax);
static __inline double clamp_(double x);

#if DEBUG
static FILE *debug_stream;
#define dbg_report(...) do { if (debug_stream) { fprintf(debug_stream, __VA_ARGS__); fflush(debug_stream); } } while(0);
#else
#define dbg_report(...)
#endif

typedef enum npclient_status_ {
    NPCLIENT_STATUS_OK,
    NPCLIENT_STATUS_DISABLED
} npclient_status;

static HANDLE hFTMemMap = 0;
static FTMemMap volatile * pMemData = 0;
static bool bEncryption = false;
static unsigned char table[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

typedef struct tir_data
{
    short status;
    short frame;
    unsigned cksum;
    float roll, pitch, yaw;
    float tx, ty, tz;
    float padding[9];
} tir_data_t;

typedef struct tir_signature
{
    char DllSignature[200];
    char AppSignature[200];
} tir_signature_t;

BOOL DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    UNUSED(lpvReserved);

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
#if DEBUG
        debug_stream = fopen("c:\\NPClient.log", "a");
#endif
#ifdef _WIN64
        dbg_report("\n= WIN64 =========================================================================================\n");
#else
        dbg_report("\n= WIN32 =========================================================================================\n");
#endif
        dbg_report("DllMain: (%p, %ld, %p)\n", (void*) hinstDLL, (long) fdwReason, lpvReserved);
        dbg_report("DllMain: Attach request\n");
        DisableThreadLibraryCalls(hinstDLL);
#if 0
        timeBeginPeriod(1);
#endif
        break;

    case DLL_PROCESS_DETACH:
        dbg_report("DllMain: Detach\n");
        dbg_report("DllMain: (%p, %ld, %p)\n", (void*) hinstDLL, (long) fdwReason, lpvReserved);
        dbg_report("==========================================================================================\n");
#if 0
        timeEndPeriod(1);
#endif
        FTDestroyMapping();
        break;
    }
    return TRUE;
}
/******************************************************************
 *              NPPriv_ClientNotify (NPCLIENT.1)
 */

NP_EXPORT(int) NPPriv_ClientNotify(void)
{
    dbg_report("stub\n");
    /* @stub in .spec */
    return 0;
}
/******************************************************************
 *              NPPriv_GetLastError (NPCLIENT.2)
 */

NP_EXPORT(int) NPPriv_GetLastError(void)
{
    dbg_report("stub\n");
    /* @stub in .spec */
    return 0;
}
/******************************************************************
 *              NPPriv_SetData (NPCLIENT.3)
 */

NP_EXPORT(int) NPPriv_SetData(void)
{
    dbg_report("stub\n");
    /* @stub in .spec */
    return 0;
}
/******************************************************************
 *              NPPriv_SetLastError (NPCLIENT.4)
 */

NP_EXPORT(int) NPPriv_SetLastError(void)
{
    dbg_report("stub\n");
    /* @stub in .spec */
    return 0;
}
/******************************************************************
 *              NPPriv_SetParameter (NPCLIENT.5)
 */

NP_EXPORT(int) NPPriv_SetParameter(void)
{
    dbg_report("stub\n");
    /* @stub in .spec */
    return 0;
}
/******************************************************************
 *              NPPriv_SetSignature (NPCLIENT.6)
 */

NP_EXPORT(int) NPPriv_SetSignature(void)
{
    dbg_report("stub\n");
    /* @stub in .spec */
    return 0;
}
/******************************************************************
 *              NPPriv_SetVersion (NPCLIENT.7)
 */

NP_EXPORT(int) NPPriv_SetVersion(void)
{
    dbg_report("stub\n");
    /* @stub in .spec */
    return 0;
}

/* TIR5 requires a checksum to be calculated over the headpose-data and to be relayed to the game. */
static unsigned cksum(unsigned char buf[], unsigned size)
{
    int rounds = size >> 2;
    int rem = size % 4;

    int c = size;
    int a0, a2;

    if (size == 0 || buf == NULL)
        return 0;

    while (rounds != 0)
    {
        a0 = *(short int*)buf;
        a2 = *(short int*)(buf+2);
        buf += 4;
        c += a0;
        a2 ^= (c << 5);
        a2 <<= 11;
        c ^= a2;
        c += (c >> 11);
        --rounds;
    }
    switch (rem)
    {
    case 3:
        a0 = *(short int*)buf;
        a2 = *(signed char*)(buf+2);
        c += a0;
        a2 = (a2 << 2) ^ c;
        c ^= (a2 << 16);
        a2 = (c >> 11);
        break;
    case 2:
        a2 = *(short int*)buf;
        c += a2;
        c ^= (c << 11);
        a2 = (c >> 17);
        break;
    case 1:
        a2 = *(signed char*)(buf);
        c += a2;
        c ^= (c << 10);
        a2 = (c >> 1);
        break;
    default:
        break;
    }
    if(rem != 0)
        c+=a2;

    c ^= (c << 3);
    c += (c >> 5);
    c ^= (c << 4);
    c += (c >> 17);
    c ^= (c << 25);
    c += (c >> 6);

    return (unsigned)c;
}

static __inline void enhance(unsigned char buf[], unsigned size, unsigned char table[], unsigned table_size)
{
  unsigned table_ptr = 0;
  unsigned char var = 0x88;
  unsigned char tmp;

  if (size <= 0 || table_size <= 0 || buf == NULL || table == NULL)
    return;

  do
  {
    tmp = buf[--size];
    buf[size] = tmp ^ table[table_ptr] ^ var;
    var += size + tmp;
    ++table_ptr;

    if (table_ptr >= table_size)
      table_ptr -= table_size;
  }
  while (size != 0);
}

/******************************************************************
 *              NP_GetData (NPCLIENT.8)
 */

NP_EXPORT(int) NP_GetData(tir_data_t * data)
{
    static int frameno = 0;
    int i;
#if DEBUG
    int recv = 0;
#endif
    if (!FTCreateMapping())
    {
        dbg_report("Can't open mapping\n");
        return 0;
    }

    if (pMemData)
    {
#if DEBUG
        recv = 1;
#endif
        y = pMemData->data.Yaw   * NP_AXIS_MAX / M_PI;
        p = pMemData->data.Pitch * NP_AXIS_MAX / M_PI;
        r = pMemData->data.Roll  * NP_AXIS_MAX / M_PI;

        tx = pMemData->data.X * NP_AXIS_MAX / 500.;
        ty = pMemData->data.Y * NP_AXIS_MAX / 500.;
        tz = pMemData->data.Z * NP_AXIS_MAX / 500.;

        if (pMemData->GameId == pMemData->GameId2 && !bEncryptionChecked)
        {
            dbg_report("NP_GetData: game = %d\n", pMemData->GameId);
            bEncryptionChecked = true;
            memcpy(table, (void*)pMemData->table, 8);
            for (i = 0; i < 8; i++)
                if (table[i])
                {
                    bEncryption = true;
                    break;
                }
            dbg_report("NP_GetData: Table = %02d %02d %02d %02d %02d %02d %02d %02d\n", table[0],table[1],table[2],table[3],table[4],table[5], table[6], table[7]);
        }
    }

    frameno++;
    data->frame = frameno;

    bool running = y != 0 || p != 0 || r != 0 ||
                   tx != 0 || ty != 0 || tz != 0;

    data->status = running ? NPCLIENT_STATUS_OK : NPCLIENT_STATUS_DISABLED;
    data->cksum = 0;

    data->roll  = clamp_(r);
    data->pitch = clamp_(p);
    data->yaw   = clamp_(y);

    data->tx = clamp_(tx);
    data->ty = clamp_(ty);
    data->tz = clamp_(tz);

    for(i = 0; i < 9; ++i)
        data->padding[i] = 0.0;

#if DEBUG
    dbg_report("GetData: rotation: %d %f %f %f\n", recv, data->yaw, data->pitch, data->roll);
#endif

    data->cksum = cksum((unsigned char*)data, sizeof(tir_data_t));

    if (bEncryption)
        enhance((unsigned char*)data, sizeof(tir_data_t), table, sizeof(table));

    return running ? NPCLIENT_STATUS_OK : NPCLIENT_STATUS_DISABLED;
}
/******************************************************************
 *              NP_GetParameter (NPCLIENT.9)
 */

NP_EXPORT(int) NP_GetParameter(int arg0, int arg1)
{
    UNUSED(arg0); UNUSED(arg1);
    dbg_report("GetParameter request: %d %d\n", arg0, arg1);
    return (int) 0;
}

/******************************************************************
 *              NP_GetSignature (NPCLIENT.10)
 *
 *
 */

static volatile unsigned char part2_2[200] = {
    0xe3, 0xe5, 0x8e, 0xe8, 0x06, 0xd4, 0xab,
    0xcf, 0xfa, 0x51, 0xa6, 0x84, 0x69, 0x52,
    0x21, 0xde, 0x6b, 0x71, 0xe6, 0xac, 0xaa,
    0x16, 0xfc, 0x89, 0xd6, 0xac, 0xe7, 0xf8,
    0x7c, 0x09, 0x6a, 0x8b, 0x8b, 0x64, 0x0b,
    0x7c, 0xc3, 0x61, 0x7f, 0xc2, 0x97, 0xd3,
    0x33, 0xd9, 0x99, 0x59, 0xbe, 0xed, 0xdc,
    0x2c, 0x5d, 0x93, 0x5c, 0xd4, 0xdd, 0xdf,
    0x8b, 0xd5, 0x1d, 0x46, 0x95, 0xbd, 0x10,
    0x5a, 0xa9, 0xd1, 0x9f, 0x71, 0x70, 0xd3,
    0x94, 0x3c, 0x71, 0x5d, 0x53, 0x1c, 0x52,
    0xe4, 0xc0, 0xf1, 0x7f, 0x87, 0xd0, 0x70,
    0xa4, 0x04, 0x07, 0x05, 0x69, 0x2a, 0x16,
    0x15, 0x55, 0x85, 0xa6, 0x30, 0xc8, 0xb6,
    0x00
};


static volatile unsigned char part1_2[200] = {
    0x6d, 0x0b, 0xab, 0x56, 0x74, 0xe6, 0x1c,
    0xff, 0x24, 0xe8, 0x34, 0x8f, 0x00, 0x63,
    0xed, 0x47, 0x5d, 0x9b, 0xe1, 0xe0, 0x1d,
    0x02, 0x31, 0x22, 0x89, 0xac, 0x1f, 0xc0,
    0xbd, 0x29, 0x13, 0x23, 0x3e, 0x98, 0xdd,
    0xd0, 0x2a, 0x98, 0x7d, 0x29, 0xff, 0x2a,
    0x7a, 0x86, 0x6c, 0x39, 0x22, 0x3b, 0x86,
    0x86, 0xfa, 0x78, 0x31, 0xc3, 0x54, 0xa4,
    0x78, 0xaa, 0xc3, 0xca, 0x77, 0x32, 0xd3,
    0x67, 0xbd, 0x94, 0x9d, 0x7e, 0x6d, 0x31,
    0x6b, 0xa1, 0xc3, 0x14, 0x8c, 0x17, 0xb5,
    0x64, 0x51, 0x5b, 0x79, 0x51, 0xa8, 0xcf,
    0x5d, 0x1a, 0xb4, 0x84, 0x9c, 0x29, 0xf0,
    0xe6, 0x69, 0x73, 0x66, 0x0e, 0x4b, 0x3c,
    0x7d, 0x99, 0x8b, 0x4e, 0x7d, 0xaf, 0x86,
    0x92, 0xff
};

static volatile unsigned char part2_1[200] = {
    0x8b, 0x84, 0xfc, 0x8c, 0x71, 0xb5, 0xd9,
    0xaa, 0xda, 0x32, 0xc7, 0xe9, 0x0c, 0x20,
    0x40, 0xd4, 0x4b, 0x02, 0x89, 0xca, 0xde,
    0x61, 0x9d, 0xfb, 0xb3, 0x8c, 0x97, 0x8a,
    0x13, 0x6a, 0x0f, 0xf8, 0xf8, 0x0d, 0x65,
    0x1b, 0xe3, 0x05, 0x1e, 0xb6, 0xf6, 0xd9,
    0x13, 0xad, 0xeb, 0x38, 0xdd, 0x86, 0xfc,
    0x59, 0x2e, 0xf6, 0x2e, 0xf4, 0xb0, 0xb0,
    0xfd, 0xb0, 0x70, 0x23, 0xfb, 0xc9, 0x1a,
    0x50, 0x89, 0x92, 0xf0, 0x01, 0x09, 0xa1,
    0xfd, 0x5b, 0x19, 0x29, 0x73, 0x59, 0x2b,
    0x81, 0x83, 0x9e, 0x11, 0xf3, 0xa2, 0x1f,
    0xc8, 0x24, 0x53, 0x60, 0x0a, 0x42, 0x78,
    0x7a, 0x39, 0xea, 0xc1, 0x59, 0xad, 0xc5,
    0x00
};

static volatile unsigned char part1_1[200] = {
    0x1d, 0x79, 0xce, 0x35, 0x1d, 0x95, 0x79,
    0xdf, 0x4c, 0x8d, 0x55, 0xeb, 0x20, 0x17,
    0x9f, 0x26, 0x3e, 0xf0, 0x88, 0x8e, 0x7a,
    0x08, 0x11, 0x52, 0xfc, 0xd8, 0x3f, 0xb9,
    0xd2, 0x5c, 0x61, 0x03, 0x56, 0xfd, 0xbc,
    0xb4, 0x0a, 0xf1, 0x13, 0x5d, 0x90, 0x0a,
    0x0e, 0xee, 0x09, 0x19, 0x45, 0x5a, 0xeb,
    0xe3, 0xf0, 0x58, 0x5f, 0xac, 0x23, 0x84,
    0x1f, 0xc5, 0xe3, 0xa6, 0x18, 0x5d, 0xb8,
    0x47, 0xdc, 0xe6, 0xf2, 0x0b, 0x03, 0x55,
    0x61, 0xab, 0xe3, 0x57, 0xe3, 0x67, 0xcc,
    0x16, 0x38, 0x3c, 0x11, 0x25, 0x88, 0x8a,
    0x24, 0x7f, 0xf7, 0xeb, 0xf2, 0x5d, 0x82,
    0x89, 0x05, 0x53, 0x32, 0x6b, 0x28, 0x54,
    0x13, 0xf6, 0xe7, 0x21, 0x1a, 0xc6, 0xe3,
    0xe1, 0xff
};

NP_EXPORT(int) NP_GetSignature(tir_signature_t * sig)
{
    int i;
    dbg_report("GetSignature request\n");

    for (i = 0; i < 200; i++)
        sig->DllSignature[i] = part1_2[i] ^ part1_1[i];
    for (i = 0; i < 200; i++)
        sig->AppSignature[i] = part2_1[i] ^ part2_2[i];
    return 0;
}

NP_EXPORT(int) NP_QueryVersion(unsigned short * version)
{
    dbg_report("QueryVersion request\n");
    *version=0x0500;
    return 0;
}
/******************************************************************
 *              NP_ReCenter (NPCLIENT.12)
 */

NP_EXPORT(int) NP_ReCenter(void)
{
    dbg_report("ReCenter request\n");
    return 0;
}

/******************************************************************
 *              NP_RegisterProgramProfileID (NPCLIENT.13)
 */

NP_EXPORT(int) NP_RegisterProgramProfileID(unsigned short id)
{
    if (FTCreateMapping())
        pMemData->GameId = id;
    dbg_report("RegisterProgramProfileID request: %d\n", id);
    return 0;
}
/******************************************************************
 *              NP_RegisterWindowHandle (NPCLIENT.14)
 */

NP_EXPORT(int) NP_RegisterWindowHandle(HWND hwnd)
{
    UNUSED(hwnd);
    dbg_report("RegisterWindowHandle request: %p\n", (void*) hwnd);
    return (int) 0;
}
/******************************************************************
 *              NP_RequestData (NPCLIENT.15)
 */

NP_EXPORT(int) NP_RequestData(unsigned short req)
{
    UNUSED(req);
    dbg_report("RequestData request: %d\n", req);
    return (int) 0;
}
/******************************************************************
 *              NP_SetParameter (NPCLIENT.16)
 */

NP_EXPORT(int) NP_SetParameter(int arg0, int arg1)
{
    UNUSED(arg0); UNUSED(arg1);
    dbg_report("SetParameter request: %d %d\n", arg0, arg1);
    return (int) 0;
}
/******************************************************************
 *              NP_StartCursor (NPCLIENT.17)
 */

NP_EXPORT(int) NP_StartCursor(void)
{
    dbg_report("StartCursor request\n");
    return (int) 0;
}
/******************************************************************
 *              NP_StartDataTransmission (NPCLIENT.18)
 */

NP_EXPORT(int) NP_StartDataTransmission(void)
{
    dbg_report("StartDataTransmission request.\n");

    return (int) 0;
}
/******************************************************************
 *              NP_StopCursor (NPCLIENT.19)
 */

NP_EXPORT(int) NP_StopCursor(void)
{
    dbg_report("StopCursor request\n");
    return (int) 0;
}
/******************************************************************
 *              NP_StopDataTransmission (NPCLIENT.20)
 */

NP_EXPORT(int) NP_StopDataTransmission(void)
{
    return (int) 0;
}
/******************************************************************
 *              NP_UnregisterWindowHandle (NPCLIENT.21)
 */

NP_EXPORT(int) NP_UnregisterWindowHandle(void)
{
    dbg_report("UnregisterWindowHandle request\n");
    return (int) 0;
}

static bool FTCreateMapping(void)
{
    if (pMemData)
        return true;

    dbg_report("FTCreateMapping request (pMemData == NULL).\n");

    HANDLE hFTMutex = CreateMutexA(NULL, FALSE, FREETRACK_MUTEX);
    CloseHandle(hFTMutex);

    hFTMemMap = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(FTMemMap), (LPCSTR) FT_MM_DATA);
    pMemData = (FTMemMap *) MapViewOfFile(hFTMemMap, FILE_MAP_WRITE, 0, 0, sizeof(FTMemMap));
    return pMemData != NULL;
}

static void FTDestroyMapping(void)
{
    if (pMemData != NULL)
        UnmapViewOfFile((void*)pMemData);

    CloseHandle(hFTMemMap);
    pMemData = 0;
    hFTMemMap = 0;
}

static __inline double clamp(double x, double xmin, double xmax)
{
    if (x > xmax)
        return xmax;

    if (x < xmin)
        return xmin;

    return x;
}

static __inline double clamp_(double x)
{
    return clamp(x, -NP_AXIS_MAX, NP_AXIS_MAX);
}
