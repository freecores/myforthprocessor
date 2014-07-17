# 
# @(#)Makefile	1.24 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for building auth modules.

BUILDDIR = ../../../../..
PACKAGE = com.sun.security.auth.module
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#

include FILES_java.gmk


ifeq ($(PLATFORM), windows)

LIBRARY = jaas_nt
  include FILES_export_windows.gmk
  include FILES_c_windows.gmk
# We need some extra libs for windows 
  ifneq ($(LIBRARY),fdlibm)
    EXTRA_LIBS += netapi32.lib user32.lib mpr.lib
  endif #fdlibm
  # code generates errors when compiled at warning level 3
  ifeq ($(ARCH_DATA_MODEL), 64)
    COMPILER_WARNING_LEVEL=0
  endif # ARCH_DATA_MODEL
endif # windows

ifeq ($(PLATFORM), solaris)
  LIBRARY = jaas_unix
  include FILES_export_unix.gmk
  include FILES_export_solaris.gmk
  include FILES_c_unix.gmk
  include FILES_c_solaris.gmk
endif # solaris

ifeq ($(PLATFORM), linux)
  LIBRARY = jaas_unix
  include FILES_export_unix.gmk
  include FILES_c_unix.gmk
endif # linux 



JAVAHFLAGS += -jni

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

#
# JVMDI implementation lives in the VM.
#
OTHER_LDLIBS = $(JVMLIB)
