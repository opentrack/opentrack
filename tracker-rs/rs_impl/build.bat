IF DEFINED %VS120COMNTOOLS%] (
	cd "%VS120COMNTOOLS%\..\..\VC"
	) ELSE (
	cd "%VS140COMNTOOLS%\..\..\VC"
	)
vcvarsall x64 && cd %~dp0 && CL /nologo /Ox /DUNICODE /D_UNICODE /MT /I"%RSSDK_DIR%\opensource\include" ftnoir_tracker_rs_impl.cpp udp_sender.cpp "%RSSDK_DIR%\opensource\src\libpxc\libpxc.cpp" /link ADVAPI32.LIB Ws2_32.lib /SUBSYSTEM:CONSOLE /OUT:bin\opentrack-tracker-rs-impl.exe 