@echo off
chcp 65001 > nul

echo ========================================
echo ProjectDG Stable Backend API Server
echo ========================================

set "PROJECT_ROOT=D:\ProjectDG"
set "BACKEND_DIR=%PROJECT_ROOT%\Server\Backend\DGBackendApi"
set "PUBLIC_IP=61.80.6.36"

set "ASPNETCORE_ENVIRONMENT=Stable"
set "ASPNETCORE_URLS=http://0.0.0.0:8080"

echo Project Root: %PROJECT_ROOT%
echo Backend Dir: %BACKEND_DIR%
echo Public IP: %PUBLIC_IP%
echo Environment: %ASPNETCORE_ENVIRONMENT%
echo Backend Url: %ASPNETCORE_URLS%
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

echo Starting Stable Backend API...
echo.
echo Local URL:
echo http://localhost:8080
echo.
echo Public URL:
echo http://%PUBLIC_IP%:8080
echo.
echo Health Check Local:
echo http://localhost:8080/health
echo.
echo Health Check Public:
echo http://%PUBLIC_IP%:8080/health
echo.

dotnet run --no-launch-profile

echo.
echo ========================================
echo Stable Backend API process ended.
echo ========================================
echo.

pause