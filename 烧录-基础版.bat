@echo off
chcp 65001 >nul
mode con: cols=110 lines=30
title 烧录 led-0.elf 到 STM32F103（OpenOCD + ST-Link）
cd /d "%~dp0"

echo ============================================================
echo   烧录 build\Debug\led-0.elf 到 STM32F103C8T6 (ST-Link)
echo ============================================================
echo.

if not exist "build\Debug\led-0.elf" (
  echo [错误] 找不到 build\Debug\led-0.elf，请先在 CLion 里 Build 一次。
  pause
  exit /b 1
)

"D:\OpenOCD\bin\openocd.exe" -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program build/Debug/led-0.elf verify reset exit"

echo.
echo ============================================================
echo   烧录结束（上面出现 Verified OK 即成功）
echo ============================================================
echo.
echo 按任意键关闭窗口 ...
pause >nul
