REM 
REM @(#)hsdb.bat	1.5 03/01/23 11:13:42
REM 
REM Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
REM SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
REM 

%JAVA_HOME\bin\java -showversion -cp classes;lib\maf-1_0.jar;lib\jlfgr-1_0.jar sun.jvm.hotspot.HSDB %1 %2
