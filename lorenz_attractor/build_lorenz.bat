@echo off
REM Build script for Lorenz Attractor project (Windows)
REM Requires Z88DK in PATH

setlocal

set OUTPUT=build\lorenz
set SOURCES=lorenz.c galaksija.c globals.c welcome_screen.c

REM Change to script directory (for double-click usage)
cd /d %~dp0

if not exist build mkdir build

zcc +gal -create-app -pragma-redirect:fputc_cons=fputc_cons_generic -o %OUTPUT% %SOURCES%

if %ERRORLEVEL% neq 0 (
    echo Build failed.
    exit /b %ERRORLEVEL%
) else (
    echo Build succeeded.
)

endlocal
