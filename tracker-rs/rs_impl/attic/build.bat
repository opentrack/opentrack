@echo off

setlocal

IF DEFINED VS150COMNTOOLS (
    if exist "%VS150COMNTOOLS%\..\..\VC\vcvarsall.bat" (
        set vs_dir="%VS150COMNTOOLS%\..\..\VC"
        set vs_64=amd64
    ) else (
        rem it installed vcvarsall.bat here for me for some reason -sh 20160827
        set vs_dir="%VS150COMNTOOLS%\..\ide\vc"
        set vs_64=amd64
    )) else (
        set vs_dir="%VS140COMNTOOLS%\..\..\VC"
        set vs_64=amd64
        )

cd /d "%~dp0"
if %errorlevel% neq 0 exit /b 1

set rs_objs=ftnoir_tracker_rs_impl.obj libpxc.obj

set rs_subdir=ia32&set vs_var=x86
call ".\build_common.bat" || exit /b 1

set rs_subdir=amd64&set vs_var=%vs_64%
call ".\build_common.bat" || exit /b 1

exit /b 0
