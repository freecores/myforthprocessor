#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.24 03/01/23
#
# Makefile for building the JDI back-end implementation
#

BUILDDIR = ../..
LIBRARY = jdwp
PRODUCT = jpda
include $(BUILDDIR)/common/Defs.gmk

OTHER_INCLUDES = -I$(INCLUDEDIR) -I$(PLATFORM_INCLUDE) \
                 -I$(SHARE_SRC)/transport/export \
                 -I$(SHARE_SRC)/back \
                 -I$(PLATFORM_SRC)/back \
                 -I$(TMPDIR)/../com.sun.tools.jdwpgen/jdwp

ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS += $(JVMLIB)
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=0
ifeq ($(ARCH), ia64)
ifeq ($(VARIANT), OPT)
# Microsofts C Compiler generates bad code for eventHelper.c at /Ox
  CFLAGS = -MD -Zi -Gy -Op -Oitb2
endif
endif
endif # PLATFORM

ifeq ($(PLATFORM), linux)
  # fork1() is declared in libjvm.so
  OTHER_LDLIBS += -ldl $(JVMLIB)
endif # PLATFORM

ifeq ($(PLATFORM), solaris)
OTHER_LDLIBS += -ldl -lsocket
endif # PLATFORM


#
# Files to compile.
#
##  include Exportedfiles.gmk
include FILES_c.gmk

# windows has one more file...
ifeq ($(PLATFORM), windows)
  FILES_c += $(CTARGDIR)dlAlloc_md.c
endif

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

# We don't want to link against -ljava
LDLIBS_OPT = 
LDLIBS_DBG = 

#
# Add to ambient vpath so we pick up the library files
#
ifeq ($(PLATFORM), windows)
vpath %.c $(SHARE_SRC)/back:$(SHARE_SRC)/back
endif # PLATFORM
vpath %.c $(SHARE_SRC)/back:$(PLATFORM_SRC)/back




