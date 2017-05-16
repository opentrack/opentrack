@setlocal ENABLEDELAYEDEXPANSION
@echo off

call msvc

taskkill -f -im trackir.exe 2>%SystemDrive%\NUL

call:check cmake .
call:check ninja i18n
call:check "d:/cygwin64/bin/dash.exe" -c "git checkout -f -- ../*/lang/ ../*/*/lang/"
call:check ninja -j6 install
call:check "d:/cygwin64/bin/dash.exe" -c "git checkout -f -- ../*/lang/ ../*/*/lang/"

exit /b 0

:check

cmd /c "msvc d:/cygwin64/bin/nice.exe -n 20 -- %*"

if %ERRORLEVEL% EQU 0 (
    GOTO:EOF
)

rem echo error %ERRORLEVEL%
exit %ERRORLEVEL%

