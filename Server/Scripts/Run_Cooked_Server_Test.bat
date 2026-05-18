@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Run Test Cooked Dedicated Server
echo ========================================

set "PROJECT_ROOT=D:\ProjectDG"
set "ARCHIVE_DIR=%PROJECT_ROOT%\BuildOutput\TestServer"
set "SERVER_MAP=/Game/Assets/FC_MedievalMonastery_0/Maps/Map_Monastery_4km_Dawn_WP"
set "LOG_FILE=%PROJECT_ROOT%\Saved\Logs\Test_Server.log"

rem Test Server Port Range: 7789~7799
set "SERVER_PORT=7789"
set "BACKEND_URL=http://localhost:8081"

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
echo Server Port: %SERVER_PORT%
echo Backend URL: %BACKEND_URL%
echo Log File: %LOG_FILE%
echo.

if not exist "%PROJECT_ROOT%\Saved\Logs" (
    mkdir "%PROJECT_ROOT%\Saved\Logs"
)

for %%A in ("%SERVER_EXE%") do set "SERVER_DIR=%%~dpA"

pushd "%SERVER_DIR%"

"%SERVER_EXE%" "%SERVER_MAP%" -log -AbsLog="%LOG_FILE%" -FORCELOGFLUSH -port=%SERVER_PORT% -BackendUrl=%BACKEND_URL%

popd

echo.
echo ========================================
echo Test Server process ended.
echo Check log:
echo %LOG_FILE%
echo ========================================
echo.

pause