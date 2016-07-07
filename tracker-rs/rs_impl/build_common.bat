@echo off

cd /d .\bin\%rs_subdir%
if %errorlevel% neq 0 exit 1

echo --- %rs_subdir%

call %vs_dir%\vcvarsall %vs_var% && CL /c /GS- /nologo /Ox /DUNICODE /D_UNICODE /MT /I "%RSSDK_DIR%\opensource\include" ..\..\ftnoir_tracker_rs_impl.cpp "%RSSDK_DIR%\opensource\src\libpxc\libpxc.cpp" && lib /NOLOGO %rs_objs% /OUT:rs-impl.lib

if %errorlevel% neq 0 exit 1
exit 0
