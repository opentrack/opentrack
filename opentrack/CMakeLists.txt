opentrack_boilerplate(opentrack-api NO-COMPAT)
target_link_libraries(opentrack-api opentrack-compat)
target_include_directories(opentrack-api PUBLIC ${CMAKE_BINARY_DIR})
