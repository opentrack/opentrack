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
    set(opentrack-hier-pfx ".")
    set(opentrack-hier-path "/")                        # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-doc "/")                         # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-bin ".")
    set(opentrack-doc-pfx "./doc")
    set(opentrack-doc-src-pfx "./source-code")
    set(opentrack-i18n-pfx "./i18n")
    set(opentrack-i18n-path "./i18n")
elseif(WIN32)
    set(opentrack-hier-pfx "./modules")
    set(opentrack-hier-path "/modules/")                # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-doc "/doc/")                     # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-bin ".")
    set(opentrack-doc-pfx "./doc")
    set(opentrack-doc-src-pfx "./source-code")
    set(opentrack-i18n-pfx "./i18n")
    set(opentrack-i18n-path "./i18n")
    set(opentrack-hier-debug "./debug")
else()
    set(opentrack-hier-pfx "libexec/opentrack")
    set(opentrack-hier-path "/../libexec/opentrack/")   # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-doc "/share/doc/opentrack/")     # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-hier-bin "/bin")
    set(opentrack-doc-pfx "./share/doc/opentrack")
    set(opentrack-doc-src-pfx "./share/doc/opentrack/source-code")
    set(opentrack-install-rpath "${CMAKE_INSTALL_PREFIX}/${opentrack-hier-pfx}")
    set(opentrack-i18n-pfx "./share/opentrack/i18n")
    set(opentrack-i18n-path "../share/opentrack/i18n")
endif()
set(opentrack-hier-str ARCHIVE DESTINATION lib/opentrack RUNTIME DESTINATION ${opentrack-hier-pfx} LIBRARY DESTINATION ${opentrack-hier-pfx})

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
