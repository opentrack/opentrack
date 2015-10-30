SET(SDK_HYDRA "" CACHE PATH "libSixense path for Razer Hydra")
SET(SDK_HYDRA_AMD64 FALSE CACHE BOOL "whether libSixense is amd64 (else ia-32)")
if(SDK_HYDRA)
    opentrack_boilerplate(opentrack-tracker-hydra)
    target_include_directories(opentrack-tracker-hydra SYSTEM PUBLIC ${SDK_HYDRA}/include ${SDK_HYDRA}/include/sixense_utils)
    if(WIN32)
        if(MSVC)
            set(dir lib)
            set(ext lib)
        else()
            set(dir bin)
            set(ext dll)
        endif()
        target_link_libraries(opentrack-tracker-hydra
                              "${SDK_HYDRA}/${dir}/win32/release_dll/sixense.${ext}"
                              #"${SDK_HYDRA}/${dir}/win32/release_dll/sixense_utils.${ext}"
                             )
                    install(FILES "${SDK_HYDRA}/bin/win32/release_dll/sixense.dll"
                                  #"${SDK_HYDRA}/bin/win32/release_dll/sixense_utils.dll"
                            DESTINATION . ${opentrack-perms})
            else()
                    if(SDK_HYDRA_AMD64)
                            set(six4 _x64)
                    else()
                            set(six4)
                    endif()
                    if(APPLE)
                            set(under-dll _dll)
                            set(soext dylib)
                            set(plat osx)
                    else()
                            set(under-dll)
                            set(soext so)
                            set(plat linux)
                    endif()
                    install(FILES
                            "${SDK_HYDRA}/lib/${plat}${six4}/release${under-dll}/libsixense${six4}.${soext}"
                            DESTINATION .
                    )
                    target_link_libraries(opentrack-tracker-hydra
                                          "${SDK_HYDRA}/lib/${plat}${six4}/release${under-dll}/libsixense${six4}.${soext}"
                                         )
    endif()
endif()
