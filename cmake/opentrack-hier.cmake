# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
# PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

if(NOT opentrack-hier-included)
    set(opentrack-hier-included TRUE)
    set(opentrack-install-rpath "")
    if(APPLE)
        set(opentrack-hier-pfx ".")
        set(opentrack-hier-path "/")
        set(opentrack-hier-doc "/")
        set(opentrack-hier-str RUNTIME DESTINATION . LIBRARY DESTINATION .)
        set(opentrack-doc-pfx "./doc")
        set(opentrack-doc-src-pfx "./source-code")
        set(opentrack-i18n-pfx "${opentrack-hier-pfx}")
    elseif(WIN32)
        set(opentrack-hier-pfx "./modules")
        set(opentrack-hier-path "/modules/")
        set(opentrack-hier-doc "/doc/")
        set(opentrack-doc-pfx "./doc")
        set(opentrack-doc-src-pfx "./source-code")
        set(opentrack-hier-str RUNTIME DESTINATION ./modules/ LIBRARY DESTINATION ./modules/)
        set(opentrack-i18n-pfx "./i18n")
    else()
        set(opentrack-hier-pfx "libexec/opentrack")
        set(opentrack-hier-path "/../libexec/opentrack/")
        set(opentrack-hier-doc "/share/doc/opentrack/")
        set(opentrack-doc-pfx "./share/doc/opentrack")
        set(opentrack-doc-src-pfx "./share/doc/opentrack/source-code")
        set(opentrack-install-rpath "${CMAKE_INSTALL_PREFIX}/${opentrack-hier-pfx}")
        set(opentrack-hier-str ARCHIVE DESTINATION lib/opentrack LIBRARY DESTINATION ${opentrack-hier-pfx} RUNTIME DESTINATION bin)
        set(opentrack-i18n-pfx "libexec/opentrack/i18n")
    endif()

    function(opentrack_escape_string var str)
        string(REGEX REPLACE "([\$\\\"#])" "\\\\\\1" tmp__ "${str}")
            set(${var} "${tmp__}" PARENT_SCOPE)
    endfunction()

    function(opentrack_setup_refresh_install_dir)
        if((NOT CMAKE_INSTALL_PREFIX STREQUAL "") AND (NOT opentrack-doc-src-pfx STREQUAL ""))
            opentrack_escape_string(dir "${CMAKE_INSTALL_PREFIX}/${opentrack-doc-src-pfx}/")
            install(CODE "file(REMOVE_RECURSE \"${dir}\")")
        endif()
    endfunction()

    opentrack_setup_refresh_install_dir()

    set(opentrack-contrib-pfx "${opentrack-doc-pfx}/contrib")

    set(opentrack-binary-suffix "")
    if(APPLE)
        set(opentrack-binary-suffix ".bin")
    elseif(WIN32)
        set(opentrack-binary-suffix ".exe")
    endif()

endif() # include guard
