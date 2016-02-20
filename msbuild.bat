@echo off

set LASTPATH=%PATH%
set LASTLIB=%LIB%
set LASTINCLUDE=%INCLUDE%

set INCLUDE=C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\include;C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\PlatformSDK\Include
set LIB=C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\lib;C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\PlatformSDK\Lib
set PATH=C:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\bin;C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE;C:\WINDOWS\Microsoft.NET\Framework\v2.0.50727

msbuild.exe /v:n /noconlog /nologo /l:FileLogger,Microsoft.Build.Engine;logfile=msbuild.log msbuild.proj

set PATH=%LASTPATH%
set LIB=%LASTLIB%
set INCLUDE=%LASTINCLUDE%
