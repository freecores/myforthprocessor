#
# @(#)Makefile	1.21 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for native threads HPI.
#
# Note:
# The fdlibm libraries are built using special rules in Library.gmk.
#

BUILDDIR     = ../..
LIBRARY      = fdlibm
PRODUCT      = java
include $(BUILDDIR)/common/Defs.gmk

#
# Where is fdlibm in the source tree?
#
FDLIBM_SRC = $(SHARE_SRC)/native/java/lang/fdlibm

# windows compiler flags
ifeq ($(PLATFORM),windows)
  OTHER_CFLAGS =
  ifeq ($(VARIANT), DBG)
    OTHER_CFLAGS += -DLOGGING
  endif
  OTHER_CFLAGS += -Od
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=1
endif

#
# Include path.
#
OTHER_INCLUDES = -I$(FDLIBM_SRC)/include

#
# Things that must be linked in.
#
OTHER_LDLIBS =

#
# Files to compile.
#
include FILES_c.gmk

#
# Rules for the .a file.
#
include $(BUILDDIR)/common/Library.gmk

#
# Override optimization primarily for Linux-i386-egcs-1.1.1 to get
# correctness on Intel.
#
ifeq ($(PLATFORM),linux)
ifeq ($(ARCH),i586)
_OPT = -g
endif
endif

#
# Find fdlibm source files.
#
vpath %.c
vpath %.c $(FDLIBM_SRC)/src
