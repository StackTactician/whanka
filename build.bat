@echo off
setlocal

set SRC=src\main.c src\lexer.c src\parser.c src\interpreter.c src\value.c src\env.c src\error.c
set OUT=pardon.exe

echo Building Pardon language interpreter...

:: Try MSVC first (cl.exe)
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo Compiler: MSVC
    cl /W4 /O2 /Fe:%OUT% %SRC% /link /out:%OUT% >nul 2>nul
    if %errorlevel% equ 0 goto :install
)

:: Try GCC (MinGW)
where gcc >nul 2>nul
if %errorlevel% equ 0 (
    echo Compiler: GCC
    gcc -Wall -Wextra -std=c11 -O2 -o %OUT% %SRC% -lm
    if %errorlevel% equ 0 goto :install
)

:: Try Clang
where clang >nul 2>nul
if %errorlevel% equ 0 (
    echo Compiler: Clang
    clang -Wall -Wextra -std=c11 -O2 -o %OUT% %SRC% -lm
    if %errorlevel% equ 0 goto :install
)

echo ERROR: No C compiler found.
echo Install one of: Visual Studio Build Tools, MinGW-w64, or LLVM/Clang
exit /b 1

:install
echo Built: %OUT%

:: Install to user PATH
set "INSTALL_DIR=%USERPROFILE%\.pardon\bin"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
copy /Y %OUT% "%INSTALL_DIR%\%OUT%" >nul
echo Installed: %INSTALL_DIR%\%OUT%

:: Add to user PATH if not already there
echo %PATH% | findstr /i /c:"%INSTALL_DIR%" >nul 2>nul
if %errorlevel% neq 0 (
    echo Adding %INSTALL_DIR% to user PATH...
    for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v Path 2^>nul') do set "CURRENT_PATH=%%B"
    if defined CURRENT_PATH (
        setx PATH "%CURRENT_PATH%;%INSTALL_DIR%" >nul 2>nul
    ) else (
        setx PATH "%INSTALL_DIR%" >nul 2>nul
    )
    echo PATH updated. Restart your terminal for it to take effect.
) else (
    echo Already in PATH.
)

echo.
echo Done. Run from anywhere: pardon ^<file.aids^>

:: Clean up MSVC artifacts
del *.obj 2>nul

endlocal
