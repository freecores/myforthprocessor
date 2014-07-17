# @(#)compile.make	1.20 03/01/23 10:51:14
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Generic compiler settings
CPP=cl.exe

!if "$(ARCH)" == "ia64"
MACHINE=IA64
CPP_FLAGS=/nologo /W3 /WX /D "CC_INTERP" /D "_LP64" /D "IA64"
!else
CPP_FLAGS=/nologo /W3 /WX /Zi /D "IA32"
MACHINE=I386
!endif

# Generic linker settings
LINK=link.exe
LINK_FLAGS= kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib Wsock32.lib winmm.lib /nologo /machine:$(MACHINE) /opt:REF /opt:ICF,8\
 /map /debug
