@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Run Cooked Client
echo ========================================

set "PROJECT_ROOT=D:\ProjectDG"
set "ARCHIVE_DIR=%PROJECT_ROOT%\BuildOutput\Client"
set "SERVER_ADDR=61.80.6.36:7777"
set "LOG_FILE=%PROJECT_ROOT%\Saved\Logs\Cooked_Client.log"

set "CLIENT_EXE="

for /f "delims=" %%F in ('dir /b /s "%ARCHIVE_DIR%\ProjectDG.exe" 2^>nul') do (
    set "CLIENT_EXE=%%F"
    goto FOUND_CLIENT
)

for /f "delims=" %%F in ('dir /b /s "%ARCHIVE_DIR%\DG.exe" 2^>nul') do (
    set "CLIENT_EXE=%%F"
    goto FOUND_CLIENT
)

:FOUND_CLIENT

if "%CLIENT_EXE%"=="" (
    echo [ERROR] Client exe not found under:
    echo %ARCHIVE_DIR%
    echo.
    dir /b /s "%ARCHIVE_DIR%"
    pause
    exit /b 1
)

echo Client EXE: %CLIENT_EXE%
echo Server Addr: %SERVER_ADDR%
echo Log File: %LOG_FILE%
echo.

if not exist "%PROJECT_ROOT%\Saved\Logs" (
    mkdir "%PROJECT_ROOT%\Saved\Logs"
)

for %%A in ("%CLIENT_EXE%") do set "CLIENT_DIR=%%~dpA"

pushd "%CLIENT_DIR%"

"%CLIENT_EXE%" %SERVER_ADDR% -log -AbsLog="%LOG_FILE%" -FORCELOGFLUSH

popd

echo.
echo ========================================
echo Client process ended.
echo Check log:
echo %LOG_FILE%
echo ========================================
echo.

pause