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
    set(opentrack-libexec "Plugins")
    set(opentrack-runtime-libexec "/Plugins/")                        # MUST HAVE A TRAILING BACKSLASH, Used in APP
    set(opentrack-runtime-doc "/")                         # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-bin "${CMAKE_INSTALL_PREFIX}")
    set(opentrack-doc "doc")
    set(opentrack-i18n "opentrack.app/Contents/Resources") # used during install
    set(opentrack-runtime-i18n "../Resources/i18n") # used in application
    set(opentrack-install-rpath "${CMAKE_INSTALL_PREFIX}/Library")
elseif(WIN32)
    set(opentrack-libexec "modules")
    set(opentrack-runtime-libexec "/${opentrack-libexec}/")  # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-runtime-doc "/doc/")                     # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-bin ".")
    set(opentrack-doc "doc")
    set(opentrack-i18n "i18n")
    set(opentrack-runtime-i18n "./i18n")
    set(opentrack-debug "debug")
    set(opentrack-install-rpath "")
else()
    set(opentrack-libexec "libexec/opentrack")
    set(opentrack-runtime-libexec "/../${opentrack-libexec}/")   # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-runtime-doc "/../share/doc/opentrack/")         # MUST HAVE A TRAILING BACKSLASH
    set(opentrack-bin "bin")
    set(opentrack-doc "share/doc/opentrack")
    set(opentrack-install-rpath "${CMAKE_INSTALL_PREFIX}/${opentrack-libexec}")
    set(opentrack-i18n "share/opentrack/i18n")
    set(opentrack-runtime-i18n "../share/opentrack/i18n")
endif()

function(otr_escape_string var str)
    string(REGEX REPLACE "([\\\"$;])" "\\\\\\1" str "${str}")
    string(REPLACE "\n" "\\n" str "${str}")
    set(${var} "${str}" PARENT_SCOPE)
endfunction()

set(opentrack-contrib-pfx "${opentrack-doc}/contrib")

set(opentrack-binary-suffix "")
if(WIN32)
    set(opentrack-binary-suffix ".exe")
endif()

set(CMAKE_INSTALL_RPATH "${opentrack-install-rpath}")
