# @(#)Makefile	1.9 03/01/23 10:51:54
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

AOUT=../../bin/jvm.dll
default:: _build_pch_file.obj $(AOUT)

!include ../../local.make
!include $(WorkSpace)/build/windows/makefiles/release.make
!include $(WorkSpace)/build/windows/makefiles/vm.make
!include ../local.make
!include local.make

!include ../generated/Dependencies

$(AOUT): $(Obj_Files)
	sh $(WorkSpace)/build/windows/build_vm_def.sh
	$(LINK) @<<
  $(LINK_FLAGS) /out:$@ /implib:$*.lib /def:vm.def $(Obj_Files)
<<

!include $(WorkSpace)/build/windows/makefiles/shared.make

