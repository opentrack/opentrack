# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
# PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

include_guard(GLOBAL)

set(opentrack-install-rpath "")
if(APPLE)
    set(opentrack-hier-root ".")
    set(opentrack-hier-pfx ".")
    set(opentrack-hier-path ".")
    set(opentrack-hier-doc ".")
    set(opentrack-hier-bin ".")
    set(opentrack-doc-pfx "doc")
    set(opentrack-doc-src-pfx "source-code")
    set(opentrack-i18n-pfx "i18n")
elseif(WIN32)
    set(opentrack-hier-root ".")
    set(opentrack-hier-pfx "modules")
    set(opentrack-hier-path "${opentrack-hier-pfx}")
    set(opentrack-hier-doc "doc")
    set(opentrack-hier-bin ".")
    set(opentrack-doc-pfx "doc")
    set(opentrack-doc-src-pfx "source-code")
    set(opentrack-i18n-pfx "i18n")
    set(opentrack-hier-debug "debug")
else()
    set(opentrack-hier-root "..")
    set(opentrack-hier-pfx "libexec/opentrack")
    set(opentrack-hier-doc "share/doc/opentrack")
    set(opentrack-hier-bin "bin")
    set(opentrack-doc-pfx "share/doc/opentrack")
    set(opentrack-doc-src-pfx "share/doc/opentrack/source-code")
    set(opentrack-i18n-pfx "share/opentrack/i18n")
    set(opentrack-install-rpath "${CMAKE_INSTALL_PREFIX}/${opentrack-hier-pfx}")
endif()
set(opentrack-hier-path "${opentrack-hier-root}/${opentrack-hier-pfx}")
set(opentrack-hier-str RUNTIME DESTINATION ${opentrack-hier-pfx} LIBRARY DESTINATION ${opentrack-hier-pfx})

function(otr_escape_string var str)
    string(REGEX REPLACE "([^_A-Za-z0-9./:-])" "\\\\\\1" str "${str}")
    set(${var} "${str}" PARENT_SCOPE)
endfunction()

set(opentrack-contrib-pfx "${opentrack-doc-pfx}/contrib")

set(opentrack-binary-suffix "")
if(APPLE)
    set(opentrack-binary-suffix ".bin")
elseif(WIN32)
    set(opentrack-binary-suffix ".exe")
endif()

set(CMAKE_INSTALL_RPATH "${opentrack-install-rpath}")
