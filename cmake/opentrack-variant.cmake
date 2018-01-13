set(opentrack_variant "default" CACHE STRING "")
set_property(CACHE opentrack_variant PROPERTY STRINGS "default;trackmouse")

# XXX that belongs in each variant's directory!

function(otr_dist_select_variant)
    if(opentrack_variant STREQUAL "trackmouse")
        set_property(GLOBAL PROPERTY opentrack-variant "trackmouse")
        set_property(GLOBAL PROPERTY opentrack-ident "trackmouse-prototype")
        set(subprojects
            "tracker-pt"
            "proto-mouse"
            "filter-accela"
            "options"
            "api"
            "compat"
            "logic"
            "dinput"
            "gui"
            "pose-widget"
            "spline"
            "cv"
            "migration")
        set_property(GLOBAL PROPERTY opentrack-subprojects "${subprojects}")
    else()
        set_property(GLOBAL PROPERTY opentrack-variant "default")
        set_property(GLOBAL PROPERTY opentrack-ident "opentrack-2.3")
        set(subprojects
            "tracker-*"
            "proto-*"
            "filter-*"
            "ext-*"
            "options"
            "api"
            "compat"
            "logic"
            "dinput"
            "gui"
            "main"
            "x-plane-plugin"
            "csv"
            "pose-widget"
            "spline"
            "qxt-mini"
            "macosx"
            "cv"
            "migration")
        set_property(GLOBAL PROPERTY opentrack-subprojects "${subprojects}")
    endif()
endfunction()

