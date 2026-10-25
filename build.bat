@echo off
setlocal

set CONFIG=%~1
if "%CONFIG%"=="" set CONFIG=Release
set BUILD_DIR=build\cmake-%CONFIG%
set VSDEVCMD=%ProgramFiles%\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat

if exist "%VSDEVCMD%" (
    call "%VSDEVCMD%" -arch=x64 -host_arch=x64
)

cmake -S . -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG%

if errorlevel 1 exit /b %errorlevel%

cmake --build "%BUILD_DIR%"

if errorlevel 1 exit /b %errorlevel%

if exist .\Game.exe (
    .\Game.exe
)
