#
# @(#)Makefile	1.2 03/01/23 11:07:37
#
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

# set WINDBG_HOME and JDK_HOME environment variables before this make.

SAWINDBGDLL = sawindbg.dll
CPP=cl.exe
LINK32=link.exe
JAVAH=$(JDK_HOME)\bin\javah
WINDBG_INCLUDE=$(WINDBG_HOME)\sdk\inc
WINDBG_LIB=$(WINDBG_HOME)\sdk\lib\i386

# These do not need to be optimized (don't run a lot of code) and it
# will be useful to have the assertion checks in place

CFLAGS=/nologo /MD /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c

LIBS= $(WINDBG_LIB)\dbgeng.lib  \
     /nologo /subsystem:console /debug /machine:I386

default: $(SAWINDBGDLL)

$(SAWINDBGDLL) : sawindbg.cpp
	@ $(JAVAH) -jni -classpath ..\..\..\..\share\vm\agent\classes sun.jvm.hotspot.debugger.windbg.WindbgDebuggerLocal 
	@ $(CPP) /I$(JDK_HOME)\include /I$(JDK_HOME)\include\win32 /I$(WINDBG_INCLUDE) $(CFLAGS) /o sawindbg.obj sawindbg.cpp
	$(LINK32) /out:$@ /DLL sawindbg.obj $(LIBS)

clean:
	rm -f *.h *.obj *.idb *.pch *.pdb *.ncb *.opt *.plg *.dll *.ilk *.exp *.lib

