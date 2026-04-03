include_guard(GLOBAL)
include(GetGitRevisionDescription)

find_package(Git QUIET)
if(GIT_FOUND)
    git_describe(OPENTRACK_COMMIT --tags --always)
endif()

set(_ver_content "#define OPENTRACK_VERSION \"${OPENTRACK_COMMIT}\"")
set(_ver_file "${CMAKE_BINARY_DIR}/opentrack-version.hxx")
set(_ver_old "")
if(EXISTS "${_ver_file}")
    file(READ "${_ver_file}" _ver_old)
endif()
if(NOT (_ver_old STREQUAL _ver_content))
    file(WRITE "${_ver_file}" "${_ver_content}")
endif()

set(version-string "\
#ifdef __cplusplus
extern \"C\"
#else
extern
#endif

const char* const opentrack_version;
const char* const opentrack_version = \"${OPENTRACK_COMMIT}\";
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
    target_compile_options(opentrack-version PRIVATE -fno-lto)
endif()
