if(LINUX OR APPLE)
    set(SDK_XPLANE "" CACHE PATH "Path to X-Plane SDK")
    opentrack_boilerplate(opentrack-xplane-plugin NO-QT)

    if(SDK_XPLANE)
        # probably librt already included
        install(FILES ${opentrack-xplane-plugin-c} DESTINATION "${opentrack-doc-src-pfx}/opentrack-xplane-plugin")
        target_include_directories(opentrack-xplane-plugin SYSTEM PUBLIC ${SDK_XPLANE}/CHeaders ${SDK_XPLANE}/CHeaders/XPLM)
        set(begin TARGET opentrack-xplane-plugin APPEND_STRING PROPERTY)

        if(CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE)
            set_property(${begin} COMPILE_FLAGS "-O2 -pipe -fPIC -DLIN -DXPLM200 -DXPLM210")
            set_property(${begin} LINK_FLAGS "-shared -rdynamic -nodefaultlibs -undefined_warning -fPIC")
        elseif(APPLE)
            set_property(${begin} COMPILE_FLAGS "-iframework ${SDK_XPLANE}/Libraries/Mac/ -DAPL -DXPLM200 -DXPLM210 -framework XPLM -framework XPWidgets")
            set_property(${begin} LINK_FLAGS "-F${SDK_XPLANE}/Libraries/Mac/ -framework XPLM -framework XPWidgets")
        endif()

        set_target_properties(opentrack-xplane-plugin PROPERTIES
            LIBRARY_OUTPUT_NAME "opentrack.xpl"
            PREFIX ""
            SUFFIX "")

        if(UNIX AND NOT APPLE)
            target_link_libraries(opentrack-xplane-plugin rt)
        endif()
    endif()
endif()
