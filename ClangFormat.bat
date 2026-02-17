@echo off
setlocal enabledelayedexpansion
set CALLER_DIR=%cd%
set /a count=0

if not exist .\clang\format\Scripts\clang-format.exe (
    python -m venv .\clang\format
    .\clang\format\Scripts\pip.exe install clang-format==21.1.8
)

for /f "delims=" %%f in ('dir /s /b src\*.c src\*.h src\*.cpp src\*.hpp ^| findstr /v /i "\\PythonModules\\"') do (
    set /a count+=1
    echo [!count!] Formatting: %%f
    .\clang\format\Scripts\clang-format.exe -i "%%f"

    if errorlevel 1 (
        echo [ERROR] Cannot format: %%f
    )
)

echo.
cd %CALLER_DIR%
