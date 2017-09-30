#ifndef __WIN32
#   error "bad cross"
#endif

#define shm_wrapper ShmWine
#include "compat/shm.h"
#include "compat/shm.cpp"
#include "wine-shm.h"
#define OPENTRACK_NO_QT_PATH
#include "opentrack-library-path.h"
#include <cstring>

using std::strcat;

static void write_path(const char* key, const char* subkey)
{
    char dir[8192];
    dir[sizeof(dir)-1] = '\0';

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
            (void) RegSetValueExA(hkpath, subkey, 0, REG_SZ, (BYTE*) dir, strlen(dir) + 1);
            RegCloseKey(hkpath);
        }
    }
}

void create_registry_key(void) {
    write_path("Software\\NaturalPoint\\NATURALPOINT\\NPClient Location", "Path");
    write_path("Software\\Freetrack\\FreeTrackClient", "Path");
}
