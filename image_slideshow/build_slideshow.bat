@echo off
REM Build script for Image Slideshow project (Windows)
REM Requires Z88DK and Python in PATH
REM
REM Usage:
REM   1. Place images in original_images\
REM   2. Run: build_slideshow.bat

setlocal

set OUTPUT=build\slideshow
set SOURCES=c\slideshow.c c\galaksija.c c\welcome_screen.c c\images.c

REM Change to script directory (for double-click usage)
cd /d %~dp0

if not exist build mkdir build

python python\constants.py c\constants.h

if %ERRORLEVEL% neq 0 (
    echo Constants header generation failed.
    exit /b %ERRORLEVEL%
)

python python\convert_images.py original_images c --preview-dir build

if %ERRORLEVEL% neq 0 (
    echo Image conversion failed.
    exit /b %ERRORLEVEL%
)

zcc +gal -create-app -pragma-redirect:fputc_cons=fputc_cons_generic -Ic -o %OUTPUT% %SOURCES%

if %ERRORLEVEL% neq 0 (
    echo Build failed.
    exit /b %ERRORLEVEL%
)

echo Build succeeded.

python python\generate_gif.py original_images build\slideshow_preview.gif

if %ERRORLEVEL% neq 0 (
    echo GIF preview generation failed.
    exit /b %ERRORLEVEL%
)

python python\generate_video.py original_images build\slideshow_preview.mp4

if %ERRORLEVEL% neq 0 (
    echo Video preview generation failed.
    exit /b %ERRORLEVEL%
)

echo Done.

endlocal
