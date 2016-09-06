include(GetGitRevisionDescription)

find_package(Git QUIET)
if(GIT_FOUND)
    git_describe(OPENTRACK_COMMIT --tags --always --dirty)
endif()

unset(_build_type)
if(CMAKE_BUILD_TYPE)
    string(TOUPPER ${CMAKE_BUILD_TYPE} _build_type)
    if (NOT _build_type STREQUAL "DEBUG")
        unset(_build_type)
    else()
        set(_build_type "-${_build_type}")
    endif()
endif()

file(WRITE ${CMAKE_BINARY_DIR}/opentrack-version.h "#define OPENTRACK_VERSION \"${OPENTRACK_COMMIT}${_build_type}\"")

set(version-string "
#define BUILD_compat
#include \"compat/export.hpp\"

#ifdef __cplusplus
extern \"C\"
#else
extern
#endif
OPENTRACK_COMPAT_EXPORT
const char* opentrack_version;

const char* opentrack_version = \"${OPENTRACK_COMMIT}${_build_type}\";
")

set(crapola-ver)
if(EXISTS ${CMAKE_BINARY_DIR}/version.c)
    file(READ ${CMAKE_BINARY_DIR}/version.c crapola-ver)
endif()

if(NOT (crapola-ver STREQUAL version-string))
    file(WRITE ${CMAKE_BINARY_DIR}/version.c "${version-string}")
endif()

add_library(opentrack-version STATIC ${CMAKE_BINARY_DIR}/version.c)
if(NOT MSVC)
    set_property(TARGET opentrack-version APPEND_STRING PROPERTY COMPILE_FLAGS "-fno-lto")
endif()
opentrack_compat(opentrack-version)
