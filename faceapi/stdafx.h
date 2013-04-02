#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>

#ifndef _MSC_VER

#include <inttypes.h>

typedef uint64_t u_int64_t;
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef uint8_t u_int8_t;
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <conio.h>
#include <sm_api_configure.h>
#ifdef SM_API
#   undef SM_API
#endif
#ifdef STDCALL
#   undef STDCALL
#endif

#define SM_API(type) type __declspec(dllimport) __stdcall
#define STDCALL __stdcall

#include <sm_api.h>
