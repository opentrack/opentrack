@set path=%vs_dir%;%path%
call vcvarsall %vs_64%
@if %errorlevel% neq 0 exit 1
CL /nologo /Ox /DUNICODE /D_UNICODE /DEXPORT_RS_IMPL /MT /I "%RSSDK_DIR%\opensource\include" ftnoir_tracker_rs_impl.cpp "%RSSDK_DIR%\opensource\src\libpxc\libpxc.cpp" /link ADVAPI32.LIB /DLL /OUT:bin\opentrack-tracker-rs-impl_amd64.dll