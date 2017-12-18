include(GetGitRevisionDescription)

find_package(Git QUIET)
if(GIT_FOUND)
    git_describe(OPENTRACK_COMMIT --tags --always)
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
#ifdef __cplusplus
extern \"C\"
#else
extern
#endif

const char* const opentrack_version;

const char* const opentrack_version = \"${OPENTRACK_COMMIT}${_build_type}\";
")

set(file "${CMAKE_CURRENT_BINARY_DIR}/version.cpp")
set(crapola-ver)
if(EXISTS "${file}")
    file(READ "${file}" crapola-ver)
endif()

if(NOT (crapola-ver STREQUAL version-string))
    file(WRITE "${file}" "${version-string}")
endif()

add_library(opentrack-version STATIC "${file}")

if(NOT MSVC)
    otr_prop(TARGET opentrack-version COMPILE_FLAGS "-fno-lto")
endif()
