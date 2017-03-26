if(NOT WIN32)
    set(SDK_WINE_PREFIX "" CACHE PATH "Wine install prefix")
    set(SDK_WINE_NO_WRAPPER FALSE CACHE BOOL "disable Wine wrapper -- use Wine only for X-Plane")
    if(SDK_WINE_PREFIX)
        otr_module(proto-wine)
        target_link_libraries(opentrack-proto-wine opentrack-csv)
        if(NOT SDK_WINE_NO_WRAPPER)
            set(my-rt -lrt)
            if(APPLE)
                set(my-rt)
            endif()
            file(GLOB wine-deps "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
            install(FILES ${wine-deps} DESTINATION "${opentrack-doc-src-pfx}/proto-wine")
            add_custom_command(
              OUTPUT opentrack-wrapper-wine.exe.so
              DEPENDS ${wine-deps}
              COMMAND ${SDK_WINE_PREFIX}/bin/wineg++ -g -DNOMINMAX -O2 -m32 -std=c++14 -o
                      opentrack-wrapper-wine.exe -I "${CMAKE_SOURCE_DIR}" -I "${CMAKE_BINARY_DIR}"
                      ${wine-deps}
                      ${my-rt})
            add_custom_target(wine-wrapper ALL DEPENDS opentrack-wrapper-wine.exe.so)
            add_dependencies(opentrack-proto-wine wine-wrapper)
            add_dependencies(wine-wrapper opentrack-compat)
            install(FILES "${CMAKE_CURRENT_BINARY_DIR}/opentrack-wrapper-wine.exe.so" DESTINATION ${opentrack-hier-pfx})
        endif()
    endif()
endif()
