setlocal
set PATH=%path%;d:\cygwin64\usr\i686-w64-mingw32\bin;d:\cygwin64\bin
cov-build --dir cov-int ninja %*
exit /b %ERRORLEVEL%
