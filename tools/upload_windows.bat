@echo off
setlocal EnableExtensions
cd /d "%~dp0\.."

where py >nul 2>nul
if not errorlevel 1 (
  py -3 -c "import sys; raise SystemExit(0 if sys.version_info >= (3, 9) else 1)" >nul 2>nul
  if not errorlevel 1 goto use_py
)

where python >nul 2>nul
if not errorlevel 1 (
  python -c "import sys; raise SystemExit(0 if sys.version_info >= (3, 9) else 1)" >nul 2>nul
  if not errorlevel 1 goto use_python
)

echo BLAD: Zainstaluj Python 3.9 lub nowszy i uruchom skrypt ponownie.
exit /b 1

:use_py
py -3 tools\project_cli.py upload %*
exit /b %errorlevel%

:use_python
python tools\project_cli.py upload %*
exit /b %errorlevel%
