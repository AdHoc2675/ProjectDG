@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Local Client Connect
echo ========================================

set "PROJECT_ROOT=C:\Users\KGA\Desktop\ProjectDG"
set "SERVER_ADDR=127.0.0.1:7777"

set "CLIENT_EXE="

if exist "%PROJECT_ROOT%\Binaries\Win64\ProjectDG.exe" (
    set "CLIENT_EXE=%PROJECT_ROOT%\Binaries\Win64\ProjectDG.exe"
)

echo Project Root: %PROJECT_ROOT%
echo Client EXE: %CLIENT_EXE%
echo Server Addr: %SERVER_ADDR%
echo.

if "%CLIENT_EXE%"=="" (
    echo [ERROR] Client exe not found.
    echo.
    echo Check this folder:
    echo %PROJECT_ROOT%\Binaries\Win64
    echo.
    dir "%PROJECT_ROOT%\Binaries\Win64"
    pause
    exit /b 1
)

"%CLIENT_EXE%" %SERVER_ADDR% -log

pause