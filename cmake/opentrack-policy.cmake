include_guard(GLOBAL)

set(_policies
    CMP0020
    CMP0022
    CMP0058
    CMP0028
    CMP0042
    CMP0063
    CMP0053
    CMP0011
    CMP0054
    CMP0012
    CMP0069
    CMP0063
    CMP0074
)
foreach(k ${_policies})
    if(POLICY ${k})
        cmake_policy(SET ${k} NEW)
    endif()
endforeach()

set(CMAKE_INSTALL_MESSAGE LAZY)
