#ifndef __WIN32
#   error "bad cross"
#endif

#define OPENTRACK_COMPAT_BUNDLED
#define PortableLockedShm ShmWine
#include "compat/compat.h"
#include "compat/compat.cpp"
#include "wine-shm.h"

static void write_path(const char* key, const char* subkey)
{
    char dir[8192];

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
            strcat(dir, "/");
            (void) RegSetValueExA(hkpath, subkey, 0, REG_SZ, (BYTE*) dir, strlen(dir) + 1);
            RegCloseKey(hkpath);
        }
    }
}

void create_registry_key(void) {
    write_path("Software\\NaturalPoint\\NATURALPOINT\\NPClient Location", "Path");
    write_path("Software\\Freetrack\\FreeTrackClient", "Path");
}
