if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-fno-lto -fno-fast-math -fno-finite-math-only -O0)
endif()

otr_module(compat NO-COMPAT BIN)

if(APPLE)
    set_source_files_properties(camera_names_mac.mm PROPERTIES COMPILE_FLAGS "-fno-objc-arc")
    target_sources(${self} PRIVATE camera_names_mac.mm)
    target_link_libraries(${self}  "-framework AVFoundation -framework Foundation")
endif()

if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(${self} rt)
endif()

if(WIN32)
    target_link_libraries(${self} strmiids)
endif()
