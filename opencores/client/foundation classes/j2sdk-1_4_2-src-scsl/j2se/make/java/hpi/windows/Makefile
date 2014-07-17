#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.18 03/01/23
#
# Makefile for Windows HPI DLL
#
BUILDDIR = ../../..
LIBRARY   = hpi
PRODUCT      = java
THREADDIR    = windows_threads
LIB_LOCATION = $(BINDIR)

include $(BUILDDIR)/common/Defs.gmk

# windows compiler flags
ifeq ($(PLATFORM),windows)
  ifeq ($(VARIANT),DBG)
    OTHER_CFLAGS = -DLOGGING
  endif
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=2
endif

FILES_c = \
    $(TARGDIR)linker_md.c \
    $(TARGDIR)memory_md.c \
    $(TARGDIR)monitor_md.c \
    $(TARGDIR)path_md.c \
    $(TARGDIR)socket_md.c \
    $(TARGDIR)sys_api_md.c \
    $(TARGDIR)system_md.c \
    $(TARGDIR)threads_md.c \
    $(TARGDIR)hpi.c # trailing blank required!

JVMLIB = 
JAVALIB =
OTHER_LCF = -export:DLL_Initialize
EXTRA_LIBS =


#
# Other files/flags shared between the HPIs.
#
include $(BUILDDIR)/java/hpi/hpi_common.gmk

#
# Create the thread specific libdir.
#
INIT = $(BINDIR)

#
# Rules for the .so file.
#
include $(BUILDDIR)/common/Library.gmk
