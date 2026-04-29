@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Local Dedicated Server - No Map Test
echo ========================================

set "PROJECT_ROOT=C:\Users\KGA\Desktop\ProjectDG"
set "SERVER_EXE=%PROJECT_ROOT%\Binaries\Win64\ProjectDGServer.exe"
set "LOG_FILE=%PROJECT_ROOT%\Saved\Logs\Local_DedicatedServer_NoMap.log"

echo Project Root: %PROJECT_ROOT%
echo Server EXE: %SERVER_EXE%
echo Log File: %LOG_FILE%
echo.

if not exist "%SERVER_EXE%" (
    echo [ERROR] ProjectDGServer.exe not found.
    pause
    exit /b 1
)

if not exist "%PROJECT_ROOT%\Saved\Logs" (
    mkdir "%PROJECT_ROOT%\Saved\Logs"
)

"%SERVER_EXE%" -log -port=7777 -AbsLog="%LOG_FILE%" -FORCELOGFLUSH

echo.
echo Server process ended.
echo Check log:
echo %LOG_FILE%
echo.

notepad "%LOG_FILE%"

pause