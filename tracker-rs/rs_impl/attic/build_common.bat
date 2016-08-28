@echo off

setlocal

cd /d .\bin\%rs_subdir% || exit 1

echo --- %rs_subdir%

set rice_ld=/OPT:REF /OPT:ICF=10 /DEBUG /DYNAMICBASE /NXCOMPAT /LTCG
set rice_lib=
set rice_cc=/Ox /arch:SSE2 /EHscr /fp:fast /GS- /GF /GR- /Gy /MT /Y- /Zi /W1 /GL /Zi
set libs=advapi32.lib

if not exist %vs_dir%\vcvarsall.bat exit /b 1
call %vs_dir%\vcvarsall %vs_var% || exit /b 1
if ["%libpath%"] == [""] exit /b 1

cl /c /nologo -DEXPORT_RS_IMPL -DUNICODE -D_UNICODE -MT %rice_cc% /I "%RSSDK_DIR%\opensource\include" ..\..\ftnoir_tracker_rs_impl.cpp "%RSSDK_DIR%\opensource\src\libpxc\libpxc.cpp" || exit /b 1
lib /nologo %rice_lib% %rs_objs% %libs% /OUT:rs-impl.lib || exit /b 1

exit /b 0
