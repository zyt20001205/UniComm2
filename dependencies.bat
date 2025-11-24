@echo off
setlocal enabledelayedexpansion

set "EXE_PATH=cmake-build-release-mingw64\UniComm.exe"
set "MSYS2_PATH=C:\msys64\mingw64\bin"
set "OUTPUT_PATH=3rd\misc\bin"

echo Checking for missing MSYS2 DLLs...

for /f "tokens=*" %%i in ('ntldd -R "%EXE_PATH%" ^| find "%MSYS2_PATH%"') do (
    for /f "tokens=3 delims=> " %%j in ("%%i") do (
        set "dll_path=%%j"
        echo !dll_path! copied
        copy "!dll_path!" "%OUTPUT_PATH%" >nul
    )
)

echo Missing DLLS copied to 3rd/misc/bin.
pause