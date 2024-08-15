@echo off
rem
rem Override main vars to be portable.
rem
set "VC6Path=%~dp0main"
set "VC6System=%~dp0system"
set "CommonProgramFiles(x86)=%~dp0shared"
set "CommonProgramFiles=%~dp0shared"
set "Path=%Path%;%VC6System%"
set "PathExt=.com;.exe;.bat;.cmd;.vbs;.vbe;.js;.jse;.wsf;.wsh;.msc"
set __COMPAT_LAYER=RunAsAdmin Win98

rem
rem Clear registry settings to avoid wrong paths
rem
regedit /s "%~dp0clearregistry.reg"

rem
rem Root of Visual Developer Studio Common files.
set "VSCommonDir=%VC6Path%\Common"

rem
rem Root of Visual Developer Studio installed files.
rem
set "MSDevDir=%VC6Path%\Common\MSDev98"

rem
rem Root of Visual C++ installed files.
rem
set "MSVCDir=%VC6Path%\VC98"

rem
rem VcOsDir is used to help create either a Windows 95 or Windows NT specific path.
rem
set "VcOsDir=Win95"
if "%OS%" == "Windows_NT" set "VcOsDir=WinNT"

rem
rem Setting environment for using Microsoft Visual C++ tools.
rem
if "%OS%" == "Windows_NT" set "Path=%MSDevDir%\Bin;%MSVCDir%\Bin;%VSCommonDir%\Tools\%VcOsDir%;%VSCommonDir%\Tools;%Path%"
if "%OS%" == "" set Path="%MSDevDir%\Bin";"%MSVCDir%\Bin";"%VSCommonDir%\Tools\%VcOsDir%";"%VSCommonDir%\Tools";"%WinDir%\System";"%Path%"
set "Include=%MSVCDir%\ATL\Include;%MSVCDir%\Include;%MSVCDir%\MFC\Include;%Include%"
set "Lib=%MSVCDir%\Lib;%MSVCDir%\MFC\Lib;%Lib%"

set VcOsDir=
set VSCommonDir=
