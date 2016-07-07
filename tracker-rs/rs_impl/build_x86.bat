@set path=%vs_dir%;%path%
call vcvarsall x86
@if %errorlevel% neq 0 exit 1
CL /nologo /Ox /DUNICODE /D_UNICODE /DEXPORT_RS_IMPL /MT /I "%RSSDK_DIR%\opensource\include" %~dp0\ftnoir_tracker_rs_impl.cpp "%RSSDK_DIR%\opensource\src\libpxc\libpxc.cpp" /link ADVAPI32.LIB /DLL /OUT:%~dp0\bin\opentrack-tracker-rs-impl.dll