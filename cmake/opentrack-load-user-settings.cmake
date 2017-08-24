if(WIN32)
    set(__sdk_username "$ENV{USERNAME}")
else()
    set(__sdk_username "$ENV{USER}")
endif()

if(".${__sdk_username}" STREQUAL ".")
    set(__sdk_username "(I-have-no-name)")
endif()

include(CMakeDetermineCCompiler)
include(CMakeDetermineCXXCompiler)

set(__sdk_paths_filename "${CMAKE_SOURCE_DIR}/sdk-paths-${__sdk_username}@${CMAKE_CXX_COMPILER_ID}.cmake")

if(EXISTS "${__sdk_paths_filename}")
    include("${__sdk_paths_filename}")
endif()

