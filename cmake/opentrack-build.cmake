# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
# PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "" FORCE)
endif()

set(CMAKE_BUILD_TYPE_INIT "RELEASE")

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
endif()

function(cleanup_build_dir)
    file(GLOB_RECURSE files LIST_DIRECTORIES TRUE RELATIVE "${CMAKE_BINARY_DIR}" "*")

    set(files_ "")

    set(got-install FALSE)
    set(got-install-file FALSE)
    set(got-cache FALSE)

    foreach(i ${files})
        if(i STREQUAL "CMakeCache.txt")
            set(got-cache TRUE)
        else()
            list(APPEND files_ "${CMAKE_BINARY_DIR}/${i}")
        endif()
    endforeach()

    if(NOT got-cache)
        message(FATAL_ERROR "sanity check failed")
    endif()

    # let's hope nothing bad happens
    file(REMOVE_RECURSE ${files_})

    #execute_process(COMMAND cmake . WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" OUTPUT_QUIET)
endfunction()
