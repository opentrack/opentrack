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
        "pose-widget"
        "spline"
        "cv"
        "migration")
    set_property(GLOBAL PROPERTY opentrack-subprojects "${subprojects}")
endfunction()
