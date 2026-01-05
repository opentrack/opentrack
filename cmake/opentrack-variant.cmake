include_guard(GLOBAL)

function(otr_init_variant)
    set_property(GLOBAL PROPERTY opentrack-variant "default")
    set_property(GLOBAL PROPERTY opentrack-ident "opentrack-2.3")

    set(subprojects
        "tracker-*"
        "proto-*"
        "filter-*"
        "options"
        "api"
        "compat"
        "logic"
        "input"
        "gui"
        "main"
        "x-plane-plugin"
        "csv"
        "pose-widget"
        "spline"
        "qxt-mini"
        "macosx"
        "cv"
        "migration"
        "main-window"
        "video"
        "video-*"
        "opentrack"
    )
    set_property(GLOBAL PROPERTY opentrack-subprojects "${subprojects}")
endfunction()
