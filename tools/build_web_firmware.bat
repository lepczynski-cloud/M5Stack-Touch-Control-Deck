@echo off
setlocal EnableExtensions
cd /d "%~dp0\.."

echo [1/2] Compiling M5Stack CoreS3 SE firmware...
call tools\build_windows.bat
if errorlevel 1 (
  echo ERROR: Build failed. Web firmware was not published.
  exit /b 1
)

if not exist ".pio-venv\Scripts\python.exe" (
  echo ERROR: Project Python environment missing.
  exit /b 1
)

echo [2/2] Preparing the single-image web installer firmware...
".pio-venv\Scripts\python.exe" tools\package_web_firmware.py
if errorlevel 1 (
  echo ERROR: Could not create the merged web firmware.
  exit /b 1
)

echo.
echo SUCCESS. For local tests only. The connected deployment builds firmware automatically:
echo   web\firmware\manifest.json
echo   web\firmware\m5stack-touch-control-deck.factory.bin
echo No manual commit of the generated .bin is needed.
echo.
exit /b 0
