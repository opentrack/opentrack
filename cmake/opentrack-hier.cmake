# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
# PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

include_guard(GLOBAL)

if(APPLE)
    set(opentrack-hier-pfx "Plugins")
    set(opentrack-hier-path "/Plugins/")                        # MUST HAVE A TRAILING BACKSLASH, Used in APP
    set(opentrack-hier-doc "/")                         # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-bin "${CMAKE_INSTALL_PREFIX}")
    set(opentrack-doc-pfx "./doc")
    set(opentrack-doc-src-pfx "./source-code")
    set(opentrack-i18n-pfx "opentrack.app/Contents/Resources") # used during install
    set(opentrack-i18n-path "../Resources/i18n") # used in application
    set(opentrack-install-rpath "${CMAKE_INSTALL_PREFIX}/Library")
elseif(WIN32)
    set(opentrack-hier-pfx "modules")
    set(opentrack-hier-path "/${opentrack-hier-pfx}/")  # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-doc "/doc/")                     # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-bin ".")
    set(opentrack-doc-pfx "./doc")
    set(opentrack-doc-src-pfx "./source-code")
    set(opentrack-i18n-pfx "./i18n")
    set(opentrack-i18n-path "./i18n")
    set(opentrack-hier-debug "./debug")
    set(opentrack-install-rpath "")
else()
    set(opentrack-hier-pfx "libexec/opentrack")
    set(opentrack-hier-path "/../${opentrack-hier-pfx}/")   # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-doc "/share/doc/opentrack/")         # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-bin "bin")
    set(opentrack-doc-pfx "./share/doc/opentrack")
    set(opentrack-doc-src-pfx "./share/doc/opentrack/source-code")
    set(opentrack-install-rpath "${CMAKE_INSTALL_PREFIX}/${opentrack-hier-pfx}")
    set(opentrack-i18n-pfx "./share/opentrack/i18n")
    set(opentrack-i18n-path "../share/opentrack/i18n")
endif()
set(opentrack-hier-str RUNTIME DESTINATION ${opentrack-hier-pfx} LIBRARY DESTINATION ${opentrack-hier-pfx})

function(otr_escape_string var str)
    string(REGEX REPLACE "([^_A-Za-z0-9./:-])" "\\\\\\1" str "${str}")
    set(${var} "${str}" PARENT_SCOPE)
endfunction()

set(opentrack-contrib-pfx "${opentrack-doc-pfx}/contrib")

set(opentrack-binary-suffix "")
if(WIN32)
    set(opentrack-binary-suffix ".exe")
endif()

set(CMAKE_INSTALL_RPATH "${opentrack-install-rpath}")
