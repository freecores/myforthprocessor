# @(#)release.make	1.18 03/01/23 10:51:22
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

!include compile.make

!if "$(Variant)" == "core"
CPP_FLAGS=$(CPP_FLAGS) /Ox /Os /Gy /Gf
!else 
!if "$(Variant)" == "compiler1"
CPP_FLAGS=$(CPP_FLAGS) /Ox /Os /Gy /Gf
!else
# CPP_FLAGS=$(CPP_FLAGS) /Ox /Os /Gy /Gf
CPP_FLAGS=$(CPP_FLAGS) /O2
!endif
!endif

CPP_FLAGS=$(CPP_FLAGS) /MD

RELEASE=

