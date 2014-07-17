@echo off
REM @(#)build.bat	1.18 03/01/23 10:50:59
REM 
REM Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
REM SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
REM 


REM
REM Since we don't have uname and we could be cross-compiling,
REM Use the compiler to determine which ARCH we are building
REM 
cl 2>&1 | grep "IA-64" >NUL
if %errorlevel% == 0 goto isia64
set ARCH=i486
goto end
:isia64
set ARCH=ia64
:end

if "%3" == ""          goto usage
if not "%5" == ""      goto usage
if "%1" == "core"      goto build
if "%1" == "compiler1" goto build
if "%1" == "compiler2" goto build
goto usage


:build
REM Add a DEVELOP=1 to the line below to make RELEASE builds instead of PRODUCT builds
nmake -f %2/build/windows/build.make Variant=%1 WorkSpace=%2 BootStrapDir=%3 BuildID=%4 BuildUser=%USERNAME%
goto end

:usage
echo Usage: build version workspace bootstrap_dir [build_id]
echo.
echo where version is "core", "compiler1" or "compiler2", workspace is source 
echo directory without trailing slash, bootstrap_dir is a full path to
echo a JDK in which bin/java and bin/javac are present and working, and
echo build_id is an optional build identifier displayed by java -version

:end
