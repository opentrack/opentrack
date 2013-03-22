@echo off
rem Copies required DLL files into output folder.

setlocal
set COPY=xcopy /D /S /C /I /H /R /Y 
set FILTER=find /v "File(s) copied"

echo parameters %1 en %2

set API_BIN=%1
set FTN_BIN=%2

%COPY% %API_BIN%\*.dll %FTN_BIN%\ | %FILTER%
exit /b 0

