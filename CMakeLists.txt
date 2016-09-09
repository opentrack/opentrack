set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
include(opentrack-check-build-directory)
include(opentrack-build)

project(opentrack C CXX)
cmake_minimum_required(VERSION 2.8.11)

include(opentrack-hier)
include(opentrack-word-size)
include(opentrack-policy)
include(opentrack-qt)
include(opentrack-platform)
include(opentrack-boilerplate)
include(opentrack-version)
include(opentrack-install)

set(C CMakeLists.txt)
file(GLOB opentrack-subprojects
    "tracker-*/${C}"
    "proto-*/${C}"
    "filter-*/${C}"
    "options/${C}"
    "api/${C}"
    "compat/${C}"
    "logic/${C}"
    "dinput/${C}"
    "gui/${C}"
    "x-plane-plugin/${C}"
    "csv/${C}"
    "pose-widget/${C}"
    "spline-widget/${C}"
    "qxt-mini/${C}"
    "macosx/${C}"
    "cv/${C}"
    "migration/${C}"
)

list(SORT opentrack-subprojects)

foreach(i ${opentrack-subprojects})
    get_filename_component(i ${i} DIRECTORY)
    add_subdirectory(${i})
endforeach()
