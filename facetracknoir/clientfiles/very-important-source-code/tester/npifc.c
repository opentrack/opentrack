#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "npifc.h"
#include "rest.h"


tir_signature_t ts;
HMODULE npclient;
/*
typedef int (*NP_RegisterWindowHandle_t)(HWND hwnd);
typedef int (*NP_UnregisterWindowHandle_t)(void);
typedef int (*NP_RegisterProgramProfileID_t)(unsigned short id);
typedef int (*NP_QueryVersion_t)(unsigned short *version);
typedef int (*NP_RequestData_t)(unsigned short req);
typedef int (*NP_GetSignature_t)(tir_signature_t *sig);
typedef int (*NP_GetData_t)(tir_data_t *data);
typedef int (*NP_GetParameter_t)(void);
typedef int (*NP_SetParameter_t)(void);
typedef int (*NP_StartCursor_t)(void);
typedef int (*NP_StopCursor_t)(void);
typedef int (*NP_ReCenter_t)(void);
typedef int (*NP_StartDataTransmission_t)(void);
typedef int (*NP_StopDataTransmission_t)(void);
*/
NP_RegisterWindowHandle_t NP_RegisterWindowHandle = NULL;
NP_UnregisterWindowHandle_t NP_UnregisterWindowHandle = NULL;
NP_RegisterProgramProfileID_t NP_RegisterProgramProfileID = NULL;
NP_QueryVersion_t NP_QueryVersion = NULL;
NP_RequestData_t NP_RequestData = NULL;
NP_GetSignature_t NP_GetSignature = NULL;
NP_GetData_t NP_GetData = NULL;
NP_GetParameter_t NP_GetParameter = NULL;
NP_SetParameter_t NP_SetParameter = NULL;
NP_StartCursor_t NP_StartCursor = NULL;
NP_StopCursor_t NP_StopCursor = NULL;
NP_ReCenter_t NP_ReCenter = NULL;
NP_StartDataTransmission_t NP_StartDataTransmission = NULL;
NP_StopDataTransmission_t NP_StopDataTransmission = NULL;

bool crypted = false;



unsigned char table[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

char *client_path()
{
  HKEY  hkey   = 0;
  RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\NaturalPoint\\NATURALPOINT\\NPClient Location", 0,
    KEY_QUERY_VALUE, &hkey);
  if(!hkey){
    printf("Can't open registry key\n");
    return NULL;
  }

  BYTE path[1024];
  DWORD buf_len = 1024;
  LONG result = RegQueryValueEx(hkey, "Path", NULL, NULL, path, &buf_len);
  char *full_path = NULL;
  int res = -1;
  if(result == ERROR_SUCCESS && buf_len > 0){
#ifdef FOR_WIN64
    res = asprintf(&full_path, "%s/NPClient64.dll", path);
#else
    res = asprintf(&full_path, "%s/NPClient.dll", path);
#endif
  }
  RegCloseKey(hkey);
  if(res > 0){
    return full_path;
  }else{
    return NULL;
  }
}

bool initialized = false;

bool npifc_init(HWND wnd, int id)
{
  //table[] = {0xb3, 0x16, 0x36, 0xeb, 0xb9, 0x05, 0x4f, 0xa4};
  game_desc_t gd;
  if(game_data_get_desc(id, &gd)){
    crypted = gd.encrypted;
    if(gd.encrypted){
      table[0] = (unsigned char)(gd.key1&0xff); gd.key1 >>= 8;
      table[1] = (unsigned char)(gd.key1&0xff); gd.key1 >>= 8;
      table[2] = (unsigned char)(gd.key1&0xff); gd.key1 >>= 8;
      table[3] = (unsigned char)(gd.key1&0xff); gd.key1 >>= 8;
      table[4] = (unsigned char)(gd.key2&0xff); gd.key2 >>= 8;
      table[5] = (unsigned char)(gd.key2&0xff); gd.key2 >>= 8;
      table[6] = (unsigned char)(gd.key2&0xff); gd.key2 >>= 8;
      table[7] = (unsigned char)(gd.key2&0xff); gd.key2 >>= 8;
    }
  }
  printf("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
          table[0], table[1], table[2], table[3],
          table[4], table[5], table[6], table[7]);

  char *client = client_path();
  if(client == NULL){
    printf("Couldn't obtain client path!\n");
    return false;
  }
  npclient = LoadLibrary(client);
  if(!npclient){
    printf("Can't load client %s\n", client);
    return false;
  }

  NP_RegisterWindowHandle = (NP_RegisterWindowHandle_t)GetProcAddress(npclient, "NP_RegisterWindowHandle");
  NP_UnregisterWindowHandle = (NP_UnregisterWindowHandle_t)GetProcAddress(npclient, "NP_UnregisterWindowHandle");
  NP_RegisterProgramProfileID = (NP_RegisterProgramProfileID_t)GetProcAddress(npclient, "NP_RegisterProgramProfileID");
  NP_QueryVersion = (NP_QueryVersion_t)GetProcAddress(npclient, "NP_QueryVersion");
  NP_RequestData = (NP_RequestData_t)GetProcAddress(npclient, "NP_RequestData");
  NP_GetSignature = (NP_GetSignature_t)GetProcAddress(npclient, "NP_GetSignature");
  NP_GetData = (NP_GetData_t)GetProcAddress(npclient, "NP_GetData");
  NP_GetParameter = (NP_GetParameter_t)GetProcAddress(npclient, "NP_GetParameter");
  NP_SetParameter = (NP_SetParameter_t)GetProcAddress(npclient, "NP_SetParameter");
  NP_StartCursor = (NP_StartCursor_t)GetProcAddress(npclient, "NP_StartCursor");
  NP_StopCursor = (NP_StopCursor_t)GetProcAddress(npclient, "NP_StopCursor");
  NP_ReCenter = (NP_ReCenter_t)GetProcAddress(npclient, "NP_ReCenter");
  NP_StartDataTransmission = (NP_StartDataTransmission_t)GetProcAddress(npclient, "NP_StartDataTransmission");
  NP_StopDataTransmission = (NP_StopDataTransmission_t)GetProcAddress(npclient, "NP_StopDataTransmission");
  if((NP_RegisterWindowHandle == NULL) || (NP_UnregisterWindowHandle == NULL)
     || (NP_RegisterProgramProfileID == NULL) || (NP_QueryVersion == NULL) || (NP_RequestData == NULL)
     || (NP_GetSignature == NULL) || (NP_GetData == NULL) || (NP_GetParameter == NULL)
     || (NP_SetParameter == NULL) || (NP_StartCursor == NULL) || (NP_StopCursor == NULL)
     || (NP_ReCenter == NULL) || (NP_StartDataTransmission == NULL) || (NP_StopDataTransmission == NULL)){
    printf("Couldn't bind all necessary functions!\n");
    return false;
  }
  tir_signature_t sig;
  int res;
  if((res = NP_GetSignature(&sig)) != 0){
    printf("Error retrieving signature! %d\n", res);
    return false;
  }
  printf("Dll Sig:%s\nApp Sig2:%s\n", sig.DllSignature, sig.AppSignature);
  NP_RegisterWindowHandle(wnd);
  if(NP_RegisterProgramProfileID(id) != 0){
    printf("Couldn't register profile id!\n");
    return false;
  }
  printf("Program profile registered!\n");
  NP_RequestData(65535);
  NP_StopCursor();
  NP_StartDataTransmission();
  initialized = true;
  return true;
}

void npifc_close()
{
  if(initialized){
    NP_StopDataTransmission();
    NP_StartCursor();
    NP_UnregisterWindowHandle();
  }
  initialized = false;
}

void c_encrypt(unsigned char buf[], unsigned int size,
             unsigned char code_table[], unsigned int table_size)
{
  unsigned int table_ptr = 0;
  unsigned char var = 0x88;
  unsigned char tmp;
  if((size <= 0) || (table_size <= 0) ||
     (buf == NULL) || (code_table == NULL))
     return;
  do{
    tmp = buf[--size];
    buf[size] = tmp ^ code_table[table_ptr] ^ var;
    var += size + tmp;
    ++table_ptr;
    if(table_ptr >= table_size){
      table_ptr -= table_size;
    }
  }while(size != 0);
}



void decrypt(unsigned char buf[], unsigned int size,
             unsigned char code_table[], unsigned int table_size)
{
  unsigned int table_ptr = 0;
  unsigned char var = 0x88;
  unsigned char tmp;
  if((size <= 0) || (table_size <= 0) ||
     (buf == NULL) || (code_table == NULL)){
      return;
  }
 do{
    tmp = buf[--size];
    buf[size] = tmp ^ code_table[table_ptr] ^ var;
    var += size + buf[size];
    ++table_ptr;
    if(table_ptr >= table_size){
      table_ptr -= table_size;
    }
  }while(size != 0);
}

unsigned int cksum(unsigned char buf[], unsigned int size)
{
  if((size == 0) || (buf == NULL)){
    return 0;
  }
  int rounds = size >> 2;
  int rem = size % 4;

  int c = size;
  int a0 = 0;
  int a2 = 0;

  while(rounds != 0){
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
  switch(rem){
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
  if(rem != 0){
    c+=a2;
  }

  c ^= (c << 3);
  c += (c >> 5);
  c ^= (c << 4);
  c += (c >> 17);
  c ^= (c << 25);
  c += (c >> 6);

  return (unsigned int)c;
}

int decode_frame(tir_data_t *td)
{
    //printf("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
    //      table[0], table[1], table[2], table[3],
    //      table[4], table[5], table[6], table[7]);
    unsigned int csum;
    decrypt((unsigned char*)td, sizeof(*td), table, sizeof(table));
    csum = td->cksum;
    td->cksum = 0;
    if(csum != cksum((unsigned char*)td, sizeof(*td))){
        printf("Problem with frame!\n");
        //int a0;
        //printf("Dec:  ");
        //for(a0 = 0; a0 < (int)sizeof(tir_data_t); ++a0)
        //{
        //  printf("%02X", ((unsigned char *)td)[a0]);
        //}
        //printf("\n");
        //printf("Cksum: %04X vs computed: %04X\n", csum, cksum((unsigned char*)td, sizeof(*td)));
        return -1;
    }
    //printf("Frame OK!\n");
    return 0;
}

int npifc_getdata(tir_data_t *data)
{
  int res = NP_GetData(data);
  if(crypted){
    decode_frame(data);
  }
  return res;
}

