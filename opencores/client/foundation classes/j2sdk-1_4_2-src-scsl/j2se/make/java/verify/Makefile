#
# @(#)Makefile	1.19 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build libverify.so
#

BUILDDIR = ../..
PRODUCT = java
LIBRARY = verify
include $(BUILDDIR)/common/Defs.gmk

#
# Link to libjvm so JVM_ functions can be called.
#
OTHER_LDLIBS += $(JVMLIB)

ifeq ($(PLATFORM), windows)
  #
  # Don't want JAVALIB
  #
  JAVALIB = 
  EXTRA_LIBS = 
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=2
endif

#
# Files to compile.
#
FILES_c = \
	check_code.c \
	check_format.c

#
# libverify.so needs these 2 header files (opcodes.h opcodes.length) 
# from the VM.
#
CPPFLAGS += -I$(SHARE_SRC)/javavm/include 

#
# Targets.
#
ifeq ($(PLATFORM), solaris)
  FILES_reorder += reorder-$(ARCH)
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Add to the ambient VPATH.
#
vpath %.c   $(SHARE_SRC)/native/common

clean::
	$(RM) $(OPT_OBJDIR)/$(LIBRARY).$(ARCH).mapfile 
	$(RM) $(DBG_OBJDIR)/$(LIBRARY)$(DBG_SUFFIX).$(ARCH).mapfile
