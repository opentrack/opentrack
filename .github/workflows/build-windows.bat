set path=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Program Files\CMake\bin;C:\Program Files\Git\cmd;%GITHUB_WORKSPACE%/ninja-build
"C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build/vcvars64.bat" >nul && %*
exit /b %ERRORLEVEL%
