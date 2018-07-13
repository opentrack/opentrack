otr_module(main-window BIN)
foreach(k user-interface logic pose-widget migration spline)
    target_link_libraries(${self} opentrack-${k})
endforeach()
