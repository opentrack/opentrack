/* This is free and unencumbered software released into the public domain.

 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
/* this is unfortunately necessary as the API is Windows-only */
#include <windows.h>

#ifdef __cplusplus
#  define MY_EXTERN extern "C"
#else
#  define MY_EXTERN
#define MY_EXPORT(ret) MY_EXTERN ret __declspec(dllexport) __cdecl
#endif

#pragma pack(push, 1)
typedef struct {
  __int16 status; /* zero means success, it's easy to forget! */
  __int16 frame; /* if idempotent, device paused */
  __int32 padding;
  float roll, pitch, yaw; /* divide by 8191 when received, in radians */
  float x, y, z; /* between -16383 and 16383 */
  float padding2[9];
} tir_headpose_t;
#pragma pack(pop)

typedef struct {
	HMODULE hlibrary;
	__int32 (__stdcall* tir_reghwnd)(HWND hwnd);
	__int32 (__stdcall* tir_regprogid)(__int16 progid);
	__int32 (__stdcall* tir_queryver)(__int16* version);
	__int32 (__stdcall* tir_reqdata)(__int16 bitmask);
	__int32 (__stdcall* tir_stopcurs)(void);
	__int32 (__stdcall* tir_startxmit)(void);
	__int32 (__stdcall* tir_getdata)(tir_headpose_t* pose);
	/* there are more functions, especially for cleaning up
	 * but the library is typically called on startup
	 * and unloaded on exit. if you really must add them,
	 * you can either extract them from NPClient.dll or
	 * google for it. for instance I once found symbols
	 * and some constants in Doxygen format...
	 * if you REALLY need them I can finish the damn thing
	 * but it's so much boilerplate :(
	 */
} tir_state_t;

MY_EXPORT(tir_state_t*) mytir_init(void)
{
	HKEY hkey = NULL;
	tir_state_t* ret;
	int i;
	DWORD sz = 1900;
	/* Windows paths are limited to 260 characters */
	char buf[2048];

	ret = malloc(sizeof(tir_state_t));
	if (ret == NULL)
		return NULL;
	memset(ret, 0, sizeof(tir_state_t));
	RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\NaturalPoint\\NATURALPOINT\\NPClient Location", 0, KEY_QUERY_VALUE, &hkey);
	if (!hkey)
		goto error;
	buf[0] = '\0';
	if (RegQueryValueExA(hkey, "Path", NULL, NULL, buf, &sz) != ERROR_SUCCESS)
		goto error;
	(void) RegCloseKey(hkey);
	hkey = NULL;
	buf[2047] = '\0';
	for (i = 0; buf[i]; i++)
		if (buf[i] == '/')
			buf[i] = '\\';
	/* in the registry path there's normally a trailing slash already */
#ifdef MY_BUILD_AMD64
	strcat(buf, "NPClient64.dll");
#else
	strcat(buf, "NPClient.dll");
#endif
	ret->hlibrary = LoadLibraryA(buf);
	if (ret->hlibrary == NULL)
		goto error;
	if ((ret->tir_reghwnd = GetProcAddress(ret->hlibrary, "NP_RegisterWindowHandle")) == NULL)
		goto error;
	if ((ret->tir_regprogid = GetProcAddress(ret->hlibrary, "NP_RegisterProgramProfileID")) == NULL)
		goto error;
	if ((ret->tir_queryver = GetProcAddress(ret->hlibrary, "NP_QueryVersion")) == NULL)
		goto error;
	if ((ret->tir_reqdata = GetProcAddress(ret->hlibrary, "NP_RequestData")) == NULL)
		goto error;
	if ((ret->tir_stopcurs = GetProcAddress(ret->hlibrary, "NP_StopCursor")) == NULL)
		goto error;
	if ((ret->tir_startxmit = GetProcAddress(ret->hlibrary, "NP_StartDataTransmission")) == NULL)
		goto error;
	if ((ret->tir_getdata = GetProcAddress(ret->hlibrary, "NP_GetData")) == NULL)
		goto error;
	return ret;
error:
	if (hkey)
	  (void) RegCloseKey(hkey);
	free(ret);
	/* sorry but I don't know anything about unloading DLLs. unless you do, there's a handle leak */
	/* if you unload it, no need to touch function pointers, they just become invalid */
	return NULL;
}

MY_EXPORT(void) mytir_start(tir_state_t* state, HWND window_handle)
{
	__int16 tmp;
	/* while you have to call it, can pass anything valid, like desktop handle I suppose? */
	(void) state->tir_reghwnd(window_handle);
	(void) state->tir_regprogid(1901); /* Falcon 4, an old flight simulation. Need to pass valid Id */
	(void) state->tir_queryver(&tmp); /* better to call it, even if we don't care about it! */
	(void) state->tir_reqdata(119); /* specifies which elements of the pose structure to return */
	(void) state->tir_stopcurs(); /* better to call it */
	(void) state->tir_startxmit(); /* idem */
}

MY_EXPORT(void) mytir_get_pose(tir_state_t* state, tir_headpose_t* ret)
{
	memset(ret, 0, sizeof(tir_headpose_t));
	ret->status = 1; /* if the call fails for some reason, at least we shouldn't think we have a valid pose when we don't */
	(void) state->tir_getdata(ret);
}

#define MY_DEMO

#ifdef MY_DEMO
int main(void)
{
	tir_state_t* state;
	int i;
	tir_headpose_t ret;

	state = mytir_init();
	if (!state)
		return 1;
	mytir_start(state, GetDesktopWindow());
	for (i = 0; i < 3600; i++)
	{
		mytir_get_pose(state, &ret);
		if (ret.status)
			return 1;
		printf("%f %f %f ~ %f %f %f\n", ret.yaw * 180 / 3.14 / 8191, ret.pitch * 180 / 3.14 / 8191, ret.roll * 180 / 3.14 / 8191, ret.x, ret.y, ret.z);
		Sleep(35);
	}
	return 0;
}
#endif