include_guard(GLOBAL)
include(GetGitRevisionDescription)

find_package(Git QUIET)
if(GIT_FOUND)
    git_describe(OPENTRACK_COMMIT --tags --always)
endif()

file(WRITE ${CMAKE_BINARY_DIR}/opentrack-version.hxx "#define OPENTRACK_VERSION \"${OPENTRACK_COMMIT}\"")

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
