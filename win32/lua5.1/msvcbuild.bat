@echo off
if "%ARCH%" equ "" (
	if %PROCESSOR_ARCHITECTURE%==x86 (
		set ARCH=x86
	) else (
		set ARCH=x64
	)
)
if "%VS%"=="2019" (set VERSION=16) && goto :generator
if "%VS%"=="16" (set VERSION=16) && goto :generator
if "%VS%"=="2017" (set VERSION=15) && goto :generator
if "%VS%"=="15" (set VERSION=15) && goto :generator
if "%VS%"=="2015" (set VERSION=14) && goto :generator
if "%VS%"=="14" (set VERSION=14) && goto :generator
if "%VS%"=="2013" (set VERSION=12) && goto :generator
if "%VS%"=="12" (set VERSION=12) && goto :generator
if "%VS%"=="2012" (set VERSION=11) && goto :generator
if "%VS%"=="11" (set VERSION=11) && goto :generator
if "%VS%"=="2010" (set VERSION=10) && goto :generator
if "%VS%"=="10" (set VERSION=10) && goto :generator
if "%VS%"=="2008" (set VERSION=9) && goto :generator
if "%VS%"=="9" (set VERSION=9) && goto :generator
if "%VS%"=="2005" (set VERSION=8) && goto :generator
if "%VS%"=="8" (set VERSION=8) && goto :generator
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.16.0" >nul 2>nul
if %errorlevel%==0 (set VERSION=16) && goto :generator
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.15.0" >nul 2>nul
if %errorlevel%==0 (set VERSION=15) && goto :generator
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.14.0" >nul 2>nul
if %errorlevel%==0 (set VERSION=14) && goto :generator
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.12.0" >nul 2>nul
if %errorlevel%==0 (set VERSION=12) && goto :generator
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.10.0" >nul 2>nul
if %errorlevel%==0 (set VERSION=11) && goto :generator
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.10.0" >nul 2>nul
if %errorlevel%==0 (set VERSION=10) && goto :generator
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.9.0 ">nul 2>nul
if %errorlevel%==0 (set VERSION=9) && goto :generator
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.8.0 ">nul 2>nul
if %errorlevel%==0 (set VERSION=8) && goto :generator
echo "Visual Studio is not installed"
goto :clean

:generator
set ARG_GENERATOR=-G"Visual Studio %VERSION%"
if %VERSION% leq 15 (
	if "%ARCH%" equ "x86" set ARG_ARCH="" && goto :build
	if "%ARCH%" equ "x64" set ARG_GENERATOR=%ARG_GENERATOR%" Win64" && set ARG_ARCH="" && goto :build
) else (
	if "%ARCH%" equ "x86" set ARG_ARCH=-AWin32 && goto :build
	if "%ARCH%" equ "x64" set ARG_ARCH=-Ax64 && goto :build
)
echo "Windows ARCH(%ARCH%) is invalid"
goto :clean

:build
if %errorlevel%==0 (cmake -H. -Bbuild %ARG_GENERATOR% %ARG_ARCH%)
if %errorlevel%==0 (cmake --build build --config Release)
goto :install

:install
md Lua\5.1 Lua\5.1\include Lua\5.1\lib
copy /Y build\Release\lua.exe Lua\5.1\lua.exe
copy /Y build\Release\luac.exe Lua\5.1\luac.exe
copy /Y build\Release\lua51.dll Lua\5.1\lua5.1.dll
copy /Y build\Release\lua51.dll Lua\5.1\lua51.dll

copy /Y build\Release\lua51.lib Lua\5.1\lib\lua5.1.lib
copy /Y build\Release\lua51.dll Lua\5.1\lib\lua5.1.dll
copy /Y build\Release\lua51.lib Lua\5.1\lib\lua51.lib
copy /Y build\Release\lua51.dll Lua\5.1\lib\lua51.dll

copy /Y src\lauxlib.h  Lua\5.1\include\lauxlib.h
copy /Y src\lua.h  Lua\5.1\include\lua.h
copy /Y src\luaconf.h  Lua\5.1\include\luaconf.h
copy /Y src\lualib.h Lua\5.1\include\lualib.h
copy /Y etc\lua.hpp  Lua\5.1\include\lua.hpp
explorer Lua\5.1

goto :end

:clean
if EXIST build RMDIR /S /Q build
if EXIST build RMDIR /S /Q Lua
goto :end

:end