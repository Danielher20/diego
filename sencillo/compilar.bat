@echo off
setlocal

cd /d "%~dp0"

set "GCC="

if exist "C:\msys64\ucrt64\bin\gcc.exe" set "GCC=C:\msys64\ucrt64\bin\gcc.exe"
if not defined GCC if exist "C:\msys64\mingw64\bin\gcc.exe" set "GCC=C:\msys64\mingw64\bin\gcc.exe"
if not defined GCC (
    for /f "delims=" %%G in ('where gcc 2^>nul') do (
        if not defined GCC set "GCC=%%G"
    )
)

if not defined GCC (
    echo No se encontro GCC.
    echo Instala MSYS2 o agrega GCC al PATH.
    pause
    exit /b 1
)

echo Compilando siga_sencillo.c...
"%GCC%" -std=c11 -Wall -Wextra -pedantic "siga_sencillo.c" -o "siga_sencillo.exe"

if errorlevel 1 (
    echo.
    echo Hubo errores al compilar.
    pause
    exit /b 1
)

echo.
echo Compilado correctamente: siga_sencillo.exe
pause
