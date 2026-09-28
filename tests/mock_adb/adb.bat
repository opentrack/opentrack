@echo off
if defined PYTHON (
  "%PYTHON%" "%~dp0adb.py" %*
) else (
  python "%~dp0adb.py" %*
)
