IF DEFINED %VS120COMNTOOLS%] (
	cd "%VS120COMNTOOLS%\..\..\VC"
	) ELSE (
	cd "%VS140COMNTOOLS%\..\..\VC"
	)

vcvarsall x86 && cd %~dp0 && CL /nologo /Ox /DUNICODE /D_UNICODE /DEXPORT_RS_IMPL /MT /I"%RSSDK_DIR%\opensource\include" ftnoir_tracker_rs_impl.cpp "%RSSDK_DIR%\opensource\src\libpxc\libpxc.cpp" /link ADVAPI32.LIB /DLL /OUT:bin\opentrack-tracker-rs-impl.dll