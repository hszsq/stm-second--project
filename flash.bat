@echo off
title Flash led-0.elf to STM32F103 (OpenOCD + ST-Link)
cd /d "%~dp0"

echo ============================================================
echo   Flash build\Debug\led-0.elf to STM32F103C8T6 via ST-Link
echo ============================================================
echo.

if not exist "build\Debug\led-0.elf" (
  echo [ERROR] build\Debug\led-0.elf not found. Build in CLion first.
  pause
  exit /b 1
)

set "OCD=D:\OpenOCD\bin\openocd.exe"
set "OCDSCRIPTS=D:\OpenOCD\share\openocd\scripts"

"%OCD%" -s "%OCDSCRIPTS%" -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program build/Debug/led-0.elf verify reset exit"

echo.
echo ============================================================
echo   Done. "Verified OK" above means success.
echo ============================================================
echo.
echo Press any key to close ...
pause >nul