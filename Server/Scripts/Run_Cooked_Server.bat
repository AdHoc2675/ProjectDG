@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Run Cooked Dedicated Server
echo ========================================

set "PROJECT_ROOT=D:\ProjectDG"
set "ARCHIVE_DIR=%PROJECT_ROOT%\BuildOutput\Server"
set "SERVER_MAP=/Game/Assets/FC_MedievalMonastery_0/Maps/Map_Monastery_4km_Dawn_WP"
set "LOG_FILE=%PROJECT_ROOT%\Saved\Logs\Cooked_Server.log"

set "SERVER_EXE="

for /f "delims=" %%F in ('dir /b /s "%ARCHIVE_DIR%\ProjectDGServer.exe" 2^>nul') do (
    set "SERVER_EXE=%%F"
    goto FOUND_SERVER
)

for /f "delims=" %%F in ('dir /b /s "%ARCHIVE_DIR%\DGServer.exe" 2^>nul') do (
    set "SERVER_EXE=%%F"
    goto FOUND_SERVER
)

:FOUND_SERVER

if "%SERVER_EXE%"=="" (
    echo [ERROR] Server exe not found under:
    echo %ARCHIVE_DIR%
    echo.
    dir /b /s "%ARCHIVE_DIR%"
    pause
    exit /b 1
)

echo Server EXE: %SERVER_EXE%
echo Server Map: %SERVER_MAP%
echo Log File: %LOG_FILE%
echo.

if not exist "%PROJECT_ROOT%\Saved\Logs" (
    mkdir "%PROJECT_ROOT%\Saved\Logs"
)

for %%A in ("%SERVER_EXE%") do set "SERVER_DIR=%%~dpA"

pushd "%SERVER_DIR%"

"%SERVER_EXE%" "%SERVER_MAP%" -log -AbsLog="%LOG_FILE%" -FORCELOGFLUSH -port=7777

popd

echo.
echo ========================================
echo Server process ended.
echo Check log:
echo %LOG_FILE%
echo ========================================
echo.

pause