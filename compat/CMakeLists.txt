if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-fno-lto -fno-fast-math -fno-finite-math-only -O0)
endif()

otr_module(compat NO-COMPAT BIN)

if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(${self} rt)
endif()

if(WIN32)
    target_link_libraries(${self} strmiids)
endif()
