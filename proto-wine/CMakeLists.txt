if(NOT WIN32)
    set(SDK_WINE "" CACHE BOOL "Build for Wine")
    set(no-wrapper FALSE)
    if(NOT SDK_WINE AND SDK_XPLANE)
        set(no-wrapper TRUE)
    endif()
    if(SDK_WINE OR no-wrapper)
        if(no-wrapper)
            add_definitions(-DOTR_WINE_NO_WRAPPER)
        endif()
        otr_module(proto-wine)
        target_include_directories(opentrack-proto-wine PRIVATE ${PROJECT_SOURCE_DIR}/gui) # needed for qlonglongspinbox
        if(NOT no-wrapper)
            target_link_libraries(opentrack-proto-wine PRIVATE opentrack-csv)
            set(my-rt -lrt)
            if(APPLE)
                set(my-rt)
            endif()
            file(GLOB wine-deps "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
            #install(FILES ${wine-deps} DESTINATION "${opentrack-src}/proto-wine")
            set(winegxx-multilib "-m32")
            if (NOT OPENTRACK_WINE_ARCH STREQUAL "")
                set(winegxx-multilib "${OPENTRACK_WINE_ARCH}")
            endif()
            add_custom_command(
              OUTPUT opentrack-wrapper-wine.exe.so
              DEPENDS ${wine-deps}
              COMMAND wineg++ -mconsole -g -DNOMINMAX -O2 ${winegxx-multilib} -std=c++17 -fPIC -o
                      opentrack-wrapper-wine.exe -I "${CMAKE_SOURCE_DIR}" -I "${CMAKE_BINARY_DIR}"
                      ${wine-deps} -Wall -Wextra -Wpedantic
                      ${my-rt})
            add_custom_target(wine-wrapper DEPENDS opentrack-wrapper-wine.exe.so)
            add_dependencies(opentrack-proto-wine wine-wrapper)
            add_dependencies(wine-wrapper opentrack-compat)
            install(FILES "${CMAKE_CURRENT_BINARY_DIR}/opentrack-wrapper-wine.exe.so" DESTINATION ${opentrack-libexec})
        endif()
    endif()
endif()
