IF DEFINED %VS120COMNTOOLS%] (
	chdir /d "%VS120COMNTOOLS%\..\..\VC"
	) ELSE (
	chdir /d "%VS140COMNTOOLS%\..\..\VC"
	)

vcvarsall x86 && chdir /d %~dp0 && CL /nologo /Ox /DUNICODE /D_UNICODE /DEXPORT_RS_IMPL /MT /I"%RSSDK_DIR%\opensource\include" ftnoir_tracker_rs_impl.cpp "%RSSDK_DIR%\opensource\src\libpxc\libpxc.cpp" /link ADVAPI32.LIB /DLL /OUT:bin\opentrack-tracker-rs-impl.dll