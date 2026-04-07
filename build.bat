@echo off

set CONFIG=Debug

if not exist build (
    cmake -B build -S .
)

cmake --build build --config %CONFIG%

if exist .\build\bin\%CONFIG%\engine_test.exe (
    .\build\bin\%CONFIG%\engine_test.exe
)