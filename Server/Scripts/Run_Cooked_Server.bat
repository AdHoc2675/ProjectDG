@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Run Cooked Dedicated Server
echo ========================================

set "PROJECT_ROOT=C:\Users\KGA\Desktop\ProjectDG"
set "SERVER_EXE=%PROJECT_ROOT%\BuildOutput\Server\WindowsServer\ProjectDG\Binaries\Win64\ProjectDGServer.exe"
set "MAP_NAME=/Game/Personal/DOHEE/Level/ServerTest"
set "LOG_FILE=%PROJECT_ROOT%\Saved\Logs\Cooked_DedicatedServer.log"

echo Server EXE:
echo %SERVER_EXE%
echo.

if not exist "%SERVER_EXE%" (
    echo [ERROR] ProjectDGServer.exe not found.
    echo Expected:
    echo %SERVER_EXE%
    pause
    exit /b 1
)

if not exist "%PROJECT_ROOT%\Saved\Logs" (
    mkdir "%PROJECT_ROOT%\Saved\Logs"
)

for %%A in ("%SERVER_EXE%") do set "SERVER_DIR=%%~dpA"

pushd "%SERVER_DIR%"

echo Current Directory:
cd
echo.

echo Starting Dedicated Server...
echo.

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