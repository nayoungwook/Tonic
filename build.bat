@echo off
setlocal

set CONFIG=Debug
set BUILD_DIR=build\cmake
set VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat

where cl >nul 2>nul
if errorlevel 1 (
    if exist "%VSDEVCMD%" (
        call "%VSDEVCMD%" -arch=x64 -host_arch=x64
    )
)

if not exist "%BUILD_DIR%\CMakeCache.txt" (
    cmake -S . -B "%BUILD_DIR%" -A x64
)

cmake --build "%BUILD_DIR%" --config %CONFIG%

if exist .\game (
    .\game.exe
)
