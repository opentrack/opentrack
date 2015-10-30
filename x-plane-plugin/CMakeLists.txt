opentrack_boilerplate(opentrack-xplane-plugin NO-LIBRARY)

if(LINUX OR APPLE)
    set(SDK_XPLANE "" CACHE PATH "Path to X-Plane SDK")
    if(SDK_XPLANE)
        # probably librt already included
        add_library(opentrack-xplane-plugin SHARED ${opentrack-xplane-plugin-c})
        target_include_directories(opentrack-xplane-plugin SYSTEM PUBLIC ${SDK_XPLANE}/CHeaders ${SDK_XPLANE}/CHeaders/XPLM)
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
            SET_TARGET_PROPERTIES(opentrack-xplane-plugin
                PROPERTIES LINK_FLAGS
                "-Wl,--version-script=${CMAKE_SOURCE_DIR}/x-plane-plugin/version-script.txt -shared -rdynamic -nodefaultlibs -undefined_warning -fPIC"
                COMPILE_FLAGS "-Wall -O2 -pipe -fPIC -DLIN -DXPLM200 -DXPLM210"
                LIBRARY_OUTPUT_NAME "opentrack.xpl"
                PREFIX "" SUFFIX "")
        endif()
        if(APPLE)
            SET_TARGET_PROPERTIES(opentrack-xplane-plugin PROPERTIES
                                  COMPILE_FLAGS "-iframework ${SDK_XPLANE}/Libraries/Mac/ -DAPL -DXPLM200 -DXPLM210 -framework XPLM -framework XPWidgets"
                                  LINK_FLAGS "-F${SDK_XPLANE}/Libraries/Mac/ -framework XPLM -framework XPWidgets"
                                  LIBRARY_OUTPUT_NAME "opentrack.xpl"
                                  PREFIX "" SUFFIX "")
        endif()
        if(UNIX AND NOT APPLE)
            target_link_libraries(opentrack-xplane-plugin rt)
        endif()
        install(TARGETS opentrack-xplane-plugin RUNTIME DESTINATION . LIBRARY DESTINATION .  )
    endif()
endif()
