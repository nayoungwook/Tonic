@echo off
setlocal

set ROOT=%~dp0
set CLI_RELEASE_DIR=%ROOT%build\cmake-Release
set CLI_DEBUG_DIR=%ROOT%build\cmake-Debug
set FULL_BUILD_DIR=%ROOT%build

if /I "%~1"=="all" (
    if exist "%FULL_BUILD_DIR%" (
        rmdir /s /q "%FULL_BUILD_DIR%"
    )
) else (
    if exist "%CLI_RELEASE_DIR%" (
        rmdir /s /q "%CLI_RELEASE_DIR%"
    )
    if exist "%CLI_DEBUG_DIR%" (
        rmdir /s /q "%CLI_DEBUG_DIR%"
    )
)

if exist "%ROOT%Game.exe" (
    del /q "%ROOT%Game.exe"
)

if exist "%ROOT%Game.ilk" (
    del /q "%ROOT%Game.ilk"
)

if exist "%ROOT%Game.pdb" (
    del /q "%ROOT%Game.pdb"
)
