opentrack_boilerplate(opentrack-compat NO-COMPAT BIN)

if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(opentrack-compat rt)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set_property(SOURCE nan.cpp APPEND_STRING PROPERTY COMPILE_FLAGS "-fno-lto -fno-fast-math -fno-finite-math-only -O0")
endif()
