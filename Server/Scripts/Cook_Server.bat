@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Cook Dedicated Server
echo ========================================

set "PROJECT_ROOT=D:\ProjectDG"
set "ENGINE_ROOT=D:\UnrealEngine-release"
set "UPROJECT=%PROJECT_ROOT%\ProjectDG.uproject"
set "UAT=%ENGINE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"
set "MAP_NAME=/Game/Personal/DOHEE/Level/ServerTestWorld"
set "ARCHIVE_DIR=%PROJECT_ROOT%\BuildOutput\Server"

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
-server ^
-noclient ^
-serverplatform=Win64 ^
-serverconfig=Development ^
-cook ^
-map="%MAP_NAME%" ^
-stage ^
-pak ^
-archive ^
-archivedirectory="%ARCHIVE_DIR%" ^
-utf8output

echo.
echo ========================================
echo Cook Server Finished
echo Archive Dir:
echo %ARCHIVE_DIR%
echo ========================================
echo.

pause