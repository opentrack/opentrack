@echo off
rem Copies required DLL files into output folder.

setlocal
set COPY=xcopy /D /S /C /I /H /R /Y 
set FILTER=find /v "File(s) copied"

echo parameters %1 en %2 en %3

set API_BIN=%1
set OUTDIR=%2
set CONFIG=%3

if %CONFIG%==Debug (goto Debug)
if %CONFIG%==Release (goto Release)

echo Unknown build configuration %CONFIG%
exit /b -1

:Debug
%COPY% %API_BIN% %OUTDIR%\ | %FILTER%
exit /b 0

:Release
%COPY% %API_BIN% %OUTDIR%\ | %FILTER%
exit /b 0

