#define OPENTRACK_COMPAT_BUNDLED

#ifndef __WIN32
#define __WIN32
#endif

#define PortableLockedShm ShmWine

#include "ftnoir_protocol_ft/fttypes.h"
#include "compat/compat.h"
#include "compat/compat.cpp"
#include <string.h>

void create_registry_key(void) {
    char dir[8192];
    
    if (GetCurrentDirectoryA(8192, dir) < 8190)
    {
        HKEY hkpath;
        if (RegCreateKeyExA(HKEY_CURRENT_USER,
                            "Software\\NaturalPoint\\NATURALPOINT\\NPClient Location",
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
            (void) RegSetValueExA(hkpath, "Path", 0, REG_SZ, (BYTE*) dir, strlen(dir) + 1);
            RegCloseKey(hkpath);
        }
    }
}
