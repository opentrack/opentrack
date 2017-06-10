otr_module(compat NO-COMPAT BIN)

if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(opentrack-compat rt)
endif()

if(WIN32)
    target_link_libraries(opentrack-compat winmm)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    otr_prop(SOURCE nan.cpp COMPILE_FLAGS "-fno-lto -fno-fast-math -fno-finite-math-only -O0")
endif()
