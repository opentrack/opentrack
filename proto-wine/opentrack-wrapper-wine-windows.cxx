#ifndef __WIN32
#   error "bad cross"
#endif

#define shm_wrapper ShmWine
#define __WINE_OLE2_H
// OSX sdk 10.8 build error otherwise
#undef _LIBCPP_MSVCRT

#include "compat/shm.h"
#include "compat/shm.cpp"
#include "wine-shm.h"
#include "compat/library-path.hpp"
#include <cstdlib>
#include <cstring>
#include <sysexits.h>

using std::strcat;

static void write_path(const char* key, const char* subkey, bool path)
{
    char dir[8192] {};

    if (GetCurrentDirectoryA(8192, dir) < 8190)
    {
        HKEY hkpath;
        if (RegCreateKeyExA(HKEY_CURRENT_USER,
                            key,
                            0,
                            NULL,
                            0,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hkpath,
                            NULL) == ERROR_SUCCESS)
        {
            for (int i = 0; dir[i]; i++)
                if (dir[i] == '\\')
                    dir[i] = '/';
            // there's always a leading and trailing slash
            strcat(dir, OPENTRACK_LIBRARY_PATH);
            //strcat(dir, "/");
            if (!path)
                dir[0] = '\0';
            (void) RegSetValueExA(hkpath, subkey, 0, REG_SZ, (BYTE*) dir, strlen(dir) + 1);
            RegCloseKey(hkpath);
        }
    }
}

void create_registry_key(void)
{
    bool use_freetrack, use_npclient;
    const char* env = getenv("OTR_WINE_PROTO");
    char* endptr;
    if (!env) env = "";
    int selection = strtol(env, &endptr, 10);
    if (*endptr)
        selection = 0;

    switch (selection)
    {
    default: std::exit(EX_USAGE);
    case 1: use_freetrack = true, use_npclient = false; break;
    case 2: use_freetrack = false, use_npclient = true; break;
    case 3: use_freetrack = true, use_npclient = true; break;
    }

    write_path("Software\\NaturalPoint\\NATURALPOINT\\NPClient Location", "Path", use_npclient);
    write_path("Software\\Freetrack\\FreeTrackClient", "Path", use_freetrack);
}
