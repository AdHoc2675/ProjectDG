@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Cook Client
echo ========================================

set "PROJECT_ROOT=C:\Users\KGA\Desktop\ProjectDG"
set "ENGINE_ROOT=C:\Users\KGA\Desktop\UnrealEngine-release"
set "UPROJECT=%PROJECT_ROOT%\ProjectDG.uproject"
set "UAT=%ENGINE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"
set "MAP_NAME=/Game/Personal/DOHEE/Level/ServerTest"
set "ARCHIVE_DIR=%PROJECT_ROOT%\BuildOutput\Client"

echo Project Root: %PROJECT_ROOT%
echo Engine Root: %ENGINE_ROOT%
echo UProject: %UPROJECT%
echo Map: %MAP_NAME%
echo Archive Dir: %ARCHIVE_DIR%
echo.

if not exist "%UAT%" (
    echo [ERROR] RunUAT.bat not found:
    echo %UAT%
    pause
    exit /b 1
)

if not exist "%UPROJECT%" (
    echo [ERROR] ProjectDG.uproject not found:
    echo %UPROJECT%
    pause
    exit /b 1
)

"%UAT%" BuildCookRun ^
-project="%UPROJECT%" ^
-noP4 ^
-build ^
-clientconfig=Development ^
-targetplatform=Win64 ^
-cook ^
-map="%MAP_NAME%" ^
-stage ^
-pak ^
-archive ^
-archivedirectory="%ARCHIVE_DIR%" ^
-utf8output

echo.
echo ========================================
echo Cook Client Finished
echo Archive Dir:
echo %ARCHIVE_DIR%
echo ========================================
echo.

pause