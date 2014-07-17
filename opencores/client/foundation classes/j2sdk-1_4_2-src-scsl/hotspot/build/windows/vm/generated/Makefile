# @(#)Makefile	1.30 03/01/23 10:51:51
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

!include ../../local.make
!include $(WorkSpace)/build/windows/makefiles/release.make
!include $(WorkSpace)/build/windows/makefiles/makedeps.make
!include ../local.make
!include local.make


!if "$(Variant)" == "compiler2"
default:: includeDB.current Dependencies incls/ad_$(ARCH).cpp incls/dfa_$(ARCH).cpp
!else
default:: includeDB.current Dependencies
!endif

IncludeDBs=$(WorkSpace)/src/share/vm/includeDB_core $(WorkSpace)/src/share/vm/includeDB_gc $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_parallelScavenge $(WorkSpace)/src/share/vm/gc_implementation/includeDB_gc_shared


!if "$(Variant)" == "core"
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_coreonly
!endif

!if "$(Variant)" == "compiler1"
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_ci
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_compiler1
!endif

!if "$(Variant)" == "compiler2"
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_ci
IncludeDBs=$(IncludeDBs) $(WorkSpace)/src/share/vm/includeDB_compiler2
!endif

# Note we don't generate a Visual C++ project file using MakeDeps for
# the batch build.
includeDB.current Dependencies: classes/MakeDeps.class $(IncludeDBs)
	cat $(IncludeDBs) > includeDB
	if exist incls rmdir /s /q incls
	mkdir incls
	$(MakeDepsJava) -Djava.class.path=classes MakeDeps WinGammaPlatform $(WorkSpace)/build/windows/platform_$(ARCH) includeDB $(MakeDepsOptions)
	rm -f includeDB.current
	cp includeDB includeDB.current

classes/MakeDeps.class: $(MakeDepsSources)
	if exist classes rmdir /s /q classes
	mkdir classes
	$(MakeDepsJavaC) -classpath $(WorkSpace)\src\share\tools\MakeDeps -g -d classes $(MakeDepsSources)

!if "$(Variant)" == "compiler2"

!include $(WorkSpace)/build/windows/makefiles/adlc.make

!endif

!include $(WorkSpace)/build/windows/makefiles/shared.make
