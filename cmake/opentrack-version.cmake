include(GetGitRevisionDescription)

find_package(Git QUIET)
if(GIT_FOUND)
    git_describe(OPENTRACK_COMMIT --tags --always --dirty)
    git_describe(OPENTRACK_TAG_EXACT --tag --exact)
endif()

file(WRITE ${CMAKE_BINARY_DIR}/opentrack-version.h "#define OPENTRACK_VERSION \"${OPENTRACK_COMMIT}\"")

add_library(opentrack-version STATIC ${CMAKE_BINARY_DIR}/version.cc)
opentrack_compat(opentrack-version)

set(version-string "
#define BUILD_compat
#include \"opentrack-compat/export.hpp\"

#ifdef __cplusplus
extern \"C\"
#endif
OPENTRACK_COMPAT_EXPORT
const char* opentrack_version;

const char* opentrack_version = \"${OPENTRACK_COMMIT}\";
")

set(crapola-ver)
if(EXISTS ${CMAKE_BINARY_DIR}/version.cc)
    file(READ ${CMAKE_BINARY_DIR}/version.cc crapola-ver)
endif()

if(NOT (crapola-ver STREQUAL version-string))
    file(WRITE ${CMAKE_BINARY_DIR}/version.cc "${version-string}")
endif()
