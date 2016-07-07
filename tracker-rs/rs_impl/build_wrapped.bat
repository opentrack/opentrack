@IF DEFINED VS150COMNTOOLS (
	set vs_dir="%VS150COMNTOOLS%\..\..\VC"
    set vs_64=x86_amd64
	) ELSE (
	set vs_dir="%VS140COMNTOOLS%\..\..\VC"
    set vs_64=amd64
	)

cmd /c %~dp0\build_x86.bat
cmd /c %~dp0\build_amd64.bat