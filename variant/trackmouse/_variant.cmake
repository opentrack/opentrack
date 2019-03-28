function(otr_init_variant)
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
        "spline"
        "cv"
        "migration"
        "executable"
        "pose-widget"
        "trackmouse"
    )
    set_property(GLOBAL PROPERTY opentrack-subprojects "${subprojects}")
endfunction()
