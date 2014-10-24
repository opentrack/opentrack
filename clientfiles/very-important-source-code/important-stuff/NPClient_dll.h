/*
 * NPClient.dll
 *
 * Generated from NPClient.dll by winedump.
 *
 * DO NOT SEND GENERATED DLLS FOR INCLUSION INTO WINE !
 *
 */
#ifndef __WINE_NPCLIENT_DLL_H
#define __WINE_NPCLIENT_DLL_H

#include "windef.h"
#include "wine/debug.h"
#include "winbase.h"
#include "winnt.h"

#pragma pack(1)
typedef struct tir_data{
  short status;
  short frame;
  unsigned int cksum;
  float roll, pitch, yaw;
  float tx, ty, tz;
  float padding[9];
} tir_data_t;

typedef struct tir_signature{
    char DllSignature[200];
    char AppSignature[200];
} tir_signature_t;
#pragma pack(0)


/* __stdcall NPCLIENT_NPPriv_ClientNotify(); */
/* __stdcall NPCLIENT_NPPriv_GetLastError(); */
/* __stdcall NPCLIENT_NPPriv_SetData(); */
/* __stdcall NPCLIENT_NPPriv_SetLastError(); */
/* __stdcall NPCLIENT_NPPriv_SetParameter(); */
/* __stdcall NPCLIENT_NPPriv_SetSignature(); */
/* __stdcall NPCLIENT_NPPriv_SetVersion(); */
int __stdcall NPCLIENT_NP_GetData(tir_data_t * data);
int __stdcall NPCLIENT_NP_GetParameter(int arg0, int arg1);
int __stdcall NPCLIENT_NP_GetSignature(tir_signature_t * sig);
int __stdcall NPCLIENT_NP_QueryVersion(unsigned short * version);
int __stdcall NPCLIENT_NP_ReCenter(void);
int __stdcall NPCLIENT_NP_RegisterProgramProfileID(unsigned short id);
int __stdcall NPCLIENT_NP_RegisterWindowHandle(HWND hwnd);
int __stdcall NPCLIENT_NP_RequestData(unsigned short req);
int __stdcall NPCLIENT_NP_SetParameter(int arg0, int arg1);
int __stdcall NPCLIENT_NP_StartCursor(void);
int __stdcall NPCLIENT_NP_StartDataTransmission(void);
int __stdcall NPCLIENT_NP_StopCursor(void);
int __stdcall NPCLIENT_NP_StopDataTransmission(void);
int __stdcall NPCLIENT_NP_UnregisterWindowHandle(void);



#endif	/* __WINE_NPCLIENT_DLL_H */
