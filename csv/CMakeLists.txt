opentrack_boilerplate(opentrack-csv NO-LIBRARY)
add_library(opentrack-csv STATIC ${opentrack-csv-c})
opentrack_compat(opentrack-csv)
target_link_libraries(opentrack-csv ${MY_QT_LIBS})
