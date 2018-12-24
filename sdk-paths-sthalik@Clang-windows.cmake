#
# qtbase configure line for safekeeping
#

# "../configure" -prefix d:\dev\qt-5.10.0 -no-ico -no-gif -no-libjpeg   \
# -opengl desktop -no-angle -no-fontconfig -no-harfbuzz                 \
# -nomake tests -no-mp -release -opensource -shared -confirm-license    \
# -no-freetype -force-debug-fo -make-tool make.exe -platform win32-g++

set(__depdir "${CMAKE_CURRENT_LIST_DIR}/../opentrack-depends")

function(setq name value)
    set("${name}" "${__depdir}/${value}" CACHE PATH "" FORCE)
endfunction()

set(opentrack_install-debug-info TRUE CACHE INTERNAL "" FORCE)

#setq(OpenCV_DIR "opencv/build-mingw-64")
setq(SDK_ARUCO_LIBPATH "aruco/build-mingw-w64/src/libaruco.a")

setq(EIGEN3_INCLUDE_DIR "eigen")

setq(SDK_FSUIPC "fsuipc")
setq(SDK_HYDRA "SixenseSDK")

setq(SDK_RIFT_140 "LibOVR-140/build-mingw-w64")

setq(SDK_VALVE_STEAMVR "steamvr")
setq(SDK_TOBII_EYEX "Tobii-EyeX")
setq(SDK_VJOYSTICK "vjoystick")

setq(SDK_REALSENSE "RSSDK-R2")

set(__compat
    -Wno-unknown-warning-option
    -Wno-ignored-optimization-argument
    -Wno-unused-command-line-option

    -Wall -Wextra -Wpedantic
    -Wstrict-aliasing=3
    -Wstrict-overflow=4
    -Wdelete-non-virtual-dtor
    -Wno-odr
    -Wattributes
)
set(_compat "")
foreach(k ${__compat})
    set(_compat "${_compat} ${k}")
endforeach()

set(base-flags "-Wall -Wextra -Wpedantic ${_compat}")
set(base-cxxflags "")
# WARNING: this is utter experimental nonsense
set(_cxxflags
    -Wweak-vtables
    -Wlifetime
    -Wfloat-overflow-conversion
    -Wabstract-vbase-init
    -Wduplicated-branches
    -Wduplicated-cond
    -Wnon-virtual-dtor
    -Wassign-enum
    -Watomic-implicit-seq-cst
    -Watomic-properties
    -Wbitfield-enum-conversion
    -Wc++11-narrowing
    -Wc++98-c++11-c++14-c++17-compat-pedantic
    -Wc++2a-compat-pedantic
    -Wc99-compat
    -Wc99-extensions
    -Wcast-qual
    -Wchar-subscripts
    -Wconsumed
    -Wdouble-conversion
    -Wdouble-promotion
    -Wnon-literal-null-conversion
    -Wshorten-64-to-32
    -Wdelete-non-virtual-dtor
    -Wfloat-equal
    -Wfloat-overflow-conversion
    -Wfloat-zero-conversion
    -Wformat-non-iso
    -Wformat-nonliteral
    -Wgcc-compat
    -Wignored-qualifiers
    -Wimplicit
    -Wimplicit-fallthrough
    -Winconsistent-missing-destructor-override
    -Winconsistent-missing-override
    -Winfinite-recursion
    -Wkeyword-macro
    -Wkeyword-compat
    -Wloop-analysis
    -Wmain
    -Wmethod-signatures
    -Wmismatched-tags
    -Wmissing-braces
    -Wmove
    -Woverriding-method-mismatch
    -Wpacked
    -Wpragmas
    -Wreorder
    -Wreturn-std-move
    -Wself-assign-field
    -Wself-move
    -Wshadow-field-in-constructor-modified
    -Wsometimes-uninitialized
    -Wstrict-prototypes
    -Wsuspicious-memaccess
    -Wtautological-compare
    -Wthread-safety
    -Wundefined-reinterpret-cast
    -Wundefined-var-template
    -Wunguarded-availability
    -Wunneeded-internal-declaration
    -Wunneeded-member-function
    -Wunreachable-code-aggressive
    -Wunused

    -Wmost

    -Wno-float-conversion
    -Wno-exit-time-destructors
    -Wno-deprecated
    -Wno-comma
)
set(base-cxxflags "")
foreach(k ${_cxxflags})
    set(base-cxxflags "${base-cxxflags} ${k}")
endforeach()

set(CMAKE_C_FLAGS "-std=c11 ${base-flags} ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++17 ${base-flags} ${base-cxxflags} ${CMAKE_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "-O3 -ffast-math ${CMAKE_C_FLAGS_RELEASE}")

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "-ggdb ${CMAKE_C_FLAGS_DEBUG}")
