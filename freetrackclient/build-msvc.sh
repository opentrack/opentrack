#!/bin/sh

export PATH="/bin:/usr/bin:$PATH"

case "$(uname -s 2>/dev/null)" in
*CYG*|*MING*|'') wrap= ;;
*) wrap=wine ;;
esac

c_src=".\\freetrackclient.c"
c_bin="..\\facetracknoir\\clientfiles\\freetrackclient.dll"
opt_link="-nologo -LTCG -SAFESEH:NO -OPT:REF,ICF"
opt_cl="
-nologo -arch:SSE2 -fp:fast -EHc -EH- -GL -GR- -GS- -Gw -LD -MT -O1
-Ob2 -Og -Oi -Ot -Oy -QIfist -volatile:iso -Ze -Fe\"${c_bin}\"
"

MSVC="VS140COMNTOOLS"

test -z "$MSVC" && {
    echo "uh-oh, no MSVC" >&2
    exit 1
}

sep="\&"

cd "$(dirname "$0")"

$wrap cmd.exe /C $(echo "
    del /F /Q $c_bin $sep
    call %${MSVC}%/vsvars32.bat 2>nul >nul $sep
    cl $opt_cl $c_src -link $opt_link
    " | tr '\n' ' ')
