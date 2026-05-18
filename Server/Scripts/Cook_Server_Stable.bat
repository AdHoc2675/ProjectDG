@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Cook Stable Dedicated Server
echo ========================================

set "PROJECT_ROOT=D:\ProjectDG"
set "ENGINE_ROOT=D:\UnrealEngine-release"
set "UPROJECT=%PROJECT_ROOT%\ProjectDG.uproject"
set "UAT=%ENGINE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"

set "LOBBY_MAP=/Game/Personal/DOHEE/Level/ServerTest"
set "WORLD_MAP=/Game/Assets/FC_MedievalMonastery_0/Maps/Map_Monastery_4km_Dawn_WP"
set "COOK_MAPS=%LOBBY_MAP%+%WORLD_MAP%"

set "ARCHIVE_DIR=%PROJECT_ROOT%\BuildOutput\Server"

echo Project Root: %PROJECT_ROOT%
echo Engine Root: %ENGINE_ROOT%
echo UProject: %UPROJECT%
echo Lobby Map: %LOBBY_MAP%
echo World Map: %WORLD_MAP%
echo Cook Maps: %COOK_MAPS%
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
-map="%COOK_MAPS%" ^
-stage ^
-pak ^
-archive ^
-archivedirectory="%ARCHIVE_DIR%" ^
-utf8output

echo.
echo ========================================
echo Cook Stable Server Finished
echo Archive Dir:
echo %ARCHIVE_DIR%
echo ========================================
echo.

pause