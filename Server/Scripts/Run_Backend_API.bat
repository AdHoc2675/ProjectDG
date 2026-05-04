@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Backend API Server
echo ========================================

set "PROJECT_ROOT=C:\Users\KGA\Desktop\ProjectDG"
set "BACKEND_DIR=%PROJECT_ROOT%\Server\Backend\DGBackendApi"

echo Project Root: %PROJECT_ROOT%
echo Backend Dir: %BACKEND_DIR%
echo.

if not exist "%BACKEND_DIR%\DGBackendApi.csproj" (
    echo [ERROR] DGBackendApi.csproj not found.
    echo.
    echo Check this folder:
    echo %BACKEND_DIR%
    echo.
    pause
    exit /b 1
)

cd /d "%BACKEND_DIR%"

echo Starting Backend API...
echo.
echo URL:
echo http://localhost:8080
echo.
echo Health Check:
echo http://localhost:8080/health
echo.

dotnet run

echo.
echo ========================================
echo Backend API process ended.
echo ========================================
echo.

pause