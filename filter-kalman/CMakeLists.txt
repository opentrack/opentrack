find_package(OpenCV 3.0)
opentrack_boilerplate(opentrack-filter-kalman)
target_link_libraries(opentrack-filter-kalman ${OpenCV_LIBS})
target_include_directories(opentrack-filter-kalman SYSTEM PUBLIC ${OpenCV_INCLUDE_DIRS})
