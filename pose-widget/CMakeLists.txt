opentrack_boilerplate(opentrack-pose-widget NO-LIBRARY)
opentrack_qt(opentrack-pose-widget)
add_library(opentrack-pose-widget STATIC ${opentrack-pose-widget-all})
opentrack_compat(opentrack-pose-widget)
target_include_directories(opentrack-pose-widget PUBLIC pose-widget/) # else Qt moc breaks
target_link_libraries(opentrack-pose-widget ${MY_QT_LIBS})
