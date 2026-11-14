@echo off
setlocal

echo ========================================================
echo   Compilando Simulador de Misiones ARTEMIS
echo ========================================================

if not exist bin mkdir bin

where gcc >nul 2>nul
if %errorlevel% equ 0 goto use_gcc

if exist "C:\msys64\ucrt64\bin\gcc.exe" goto use_msys2

where cl >nul 2>nul
if %errorlevel% equ 0 goto use_cl

goto find_vs

:use_gcc
echo [INFO] Compilando con GCC del sistema...
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude src/main.c src/interfaz.c src/simulador.c src/fisica.c src/validaciones.c src/reportes.c -lm -o bin/artemis.exe
goto check_result

:use_msys2
echo [INFO] Compilando con GCC de MSYS2 UCRT64...
"C:\msys64\ucrt64\bin\gcc.exe" -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude src/main.c src/interfaz.c src/simulador.c src/fisica.c src/validaciones.c src/reportes.c -lm -o bin/artemis.exe
goto check_result

:find_vs
set "VCVARS="
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

if "%VCVARS%"=="" goto no_compiler

echo [INFO] Cargando entorno de Visual Studio...
call "%VCVARS%" >nul

:use_cl
echo [INFO] Compilando con MSVC (cl.exe)...
cl.exe /nologo /std:c11 /W4 /utf-8 /D_CRT_SECURE_NO_WARNINGS /Iinclude src\main.c src\interfaz.c src\simulador.c src\fisica.c src\validaciones.c src\reportes.c /Fe:bin\artemis.exe
set "BUILD_STATUS=%errorlevel%"
if exist *.obj del /q *.obj 2>nul
if %BUILD_STATUS% equ 0 goto success
goto fail

:check_result
if %errorlevel% equ 0 goto success
goto fail

:success
echo [OK] Compilacion completada con exito: bin\artemis.exe
exit /b 0

:fail
echo [ERROR] Ocurrio un error durante la compilacion.
exit /b 1

:no_compiler
echo [ERROR] No se encontro GCC ni Visual Studio C/C++.
exit /b 1
