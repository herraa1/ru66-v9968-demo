@echo off
REM V9968_demo Build Script for Windows
REM Builds the V9968 hardware demonstration project for MSXgl

setlocal enabledelayedexpansion

REM Get the directory where this batch file is located
set SCRIPT_DIR=%~dp0

REM Navigate to MSXgl root (..\..\ from script location)
pushd "%SCRIPT_DIR%"
cd ..\..\
set MSXGL_ROOT=%cd%
popd

REM Build the project using MSXgl's build system
if exist "%MSXGL_ROOT%\tools\build\Node\node.exe" (
	pushd "%SCRIPT_DIR%"
	"%MSXGL_ROOT%\tools\build\Node\node.exe" "%MSXGL_ROOT%\engine\script\js\build.js" projname=RU66 %*
	popd
) else (
	echo Error: Could not find Node.js at %MSXGL_ROOT%\tools\build\Node\node.exe
	echo Please run this script from within an MSXgl project directory.
	exit /b 1
)
