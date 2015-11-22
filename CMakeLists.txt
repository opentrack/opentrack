project(opentrack)
cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/)
include(CMakeParseArguments)
include(opentrack-policy)
include(opentrack-qt)
include(opentrack-platform)
include(opentrack-macros)
include(opentrack-version)
include(opentrack-tarball)
include(opentrack-install)

set(C CMakeLists.txt)
file(GLOB opentrack-subprojects
    "tracker-*/${C}"
    "proto-*/${C}"
    "filter-*/${C}"
    "opentrack*/${C}"
    "gui/${C}"
    "x-plane-plugin/${C}"
    "csv/${C}"
    "pose-widget/${C}"
    "spline-widget/${C}"
    "qxt-mini/${C}"
    "macosx/${C}"
)
foreach(i ${opentrack-subprojects})
    get_filename_component(i ${i} DIRECTORY)
    get_filename_component(j ${i} NAME)
    message(STATUS "project ${j}")
    add_subdirectory(${i})
endforeach()
