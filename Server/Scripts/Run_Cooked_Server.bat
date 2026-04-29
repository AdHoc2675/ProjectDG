@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Run Cooked Dedicated Server
echo ========================================

set "PROJECT_ROOT=C:\Users\KGA\Desktop\ProjectDG"
set "ARCHIVE_DIR=%PROJECT_ROOT%\BuildOutput\Server"
set "MAP_NAME=/Game/Personal/DOHEE/Level/ServerTest"
set "LOG_FILE=%PROJECT_ROOT%\Saved\Logs\Cooked_DedicatedServer.log"

set "SERVER_EXE="

for /f "delims=" %%F in ('dir /b /s "%ARCHIVE_DIR%\ProjectDGServer.exe" 2^>nul') do (
    set "SERVER_EXE=%%F"
    goto FOUND_SERVER
)

:FOUND_SERVER

if "%SERVER_EXE%"=="" (
    echo [ERROR] ProjectDGServer.exe not found under:
    echo %ARCHIVE_DIR%
    echo.
    dir /b /s "%ARCHIVE_DIR%"
    pause
    exit /b 1
)

echo Server EXE: %SERVER_EXE%
echo Map Name: %MAP_NAME%
echo Log File: %LOG_FILE%
echo.

if not exist "%PROJECT_ROOT%\Saved\Logs" (
    mkdir "%PROJECT_ROOT%\Saved\Logs"
)

for %%A in ("%SERVER_EXE%") do set "SERVER_DIR=%%~dpA"

pushd "%SERVER_DIR%"

"%SERVER_EXE%" %MAP_NAME% -log -port=7777 -AbsLog="%LOG_FILE%" -FORCELOGFLUSH

popd

echo.
echo ========================================
echo Server process ended.
echo Check log:
echo %LOG_FILE%
echo ========================================
echo.

notepad "%LOG_FILE%"

pause