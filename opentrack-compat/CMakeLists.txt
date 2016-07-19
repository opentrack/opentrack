opentrack_boilerplate(opentrack-compat NO-COMPAT BIN)
if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(opentrack-compat rt)
endif()
if(CMAKE_COMPILER_IS_GNUCXX)
    set_source_files_properties(nan.cpp PROPERTIES
        COMPILE_FLAGS "-fno-fast-math -fno-finite-math-only -O0")
endif()
