@echo off

IF DEFINED VS150COMNTOOLS (
	set vs_dir="%VS150COMNTOOLS%\..\..\VC"
    set vs_64=x86_amd64
	) ELSE (
	set vs_dir="%VS140COMNTOOLS%\..\..\VC"
    set vs_64=amd64
	)


cd /d "%~dp0"
if %errorlevel% neq 0 exit 1

set rs_objs=ftnoir_tracker_rs_impl.obj libpxc.obj

set rs_subdir=ia32&set vs_var=x86
cmd /c "call .\build_common.bat"

set rs_subdir=amd64&set vs_var=%vs_64%
cmd /c "call .\build_common.bat"
