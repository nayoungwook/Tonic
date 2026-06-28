@echo off
setlocal

set BUILD_DIR=%~dp0build

if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
)
