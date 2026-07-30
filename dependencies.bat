@echo off
setlocal enabledelayedexpansion

set "EXE_PATH=cmake-build-debug\UniComm.exe"
set "FFMPEG_PATH=cmake-build-debug\share\qt6\plugins\multimedia\ffmpegmediaplugin.dll"
set "SQLITE_PATH=cmake-build-debug\share\qt6\plugins\sqldrivers\qsqlite.dll"
set "MSYS2_PATH=C:\msys64\ucrt64\bin"
set "PATH=%MSYS2_PATH%;%PATH%"
set "OUTPUT_PATH=3rd\misc\bin"

echo Checking for missing MSYS2 DLLs...

for /f "tokens=*" %%i in ('ntldd -R "%EXE_PATH%" ^| find "%MSYS2_PATH%"') do (
    for /f "tokens=3 delims=> " %%j in ("%%i") do (
        set "dll_path=%%j"
        echo !dll_path! copied
        copy "!dll_path!" "%OUTPUT_PATH%" >nul
    )
)

for /f "tokens=*" %%i in ('ntldd -R "%FFMPEG_PATH%" ^| find "%MSYS2_PATH%"') do (
    for /f "tokens=3 delims=> " %%j in ("%%i") do (
        set "dll_path=%%j"
        echo !dll_path! copied
        copy "!dll_path!" "%OUTPUT_PATH%" >nul
    )
)

for /f "tokens=*" %%i in ('ntldd -R "%SQLITE_PATH%" ^| find "%MSYS2_PATH%"') do (
    for /f "tokens=3 delims=> " %%j in ("%%i") do (
        set "dll_path=%%j"
        echo !dll_path! copied
        copy "!dll_path!" "%OUTPUT_PATH%" >nul
    )
)

echo Missing DLLS copied to 3rd/misc/bin.
pause