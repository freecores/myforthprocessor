#
# @(#)Makefile	1.14 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


BUILDDIR = ../..
PACKAGE = sun.jdga
LIBRARY = sunwjdga
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
FILES_c = \
	dgalock.c

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)

ifneq ($(PLATFORM), windows)
  LDLIBS =  -ldga -lX11 -ldl -lc

  CPPFLAGS += \
	-I$(SHARE_SRC)/javavm/export \
	-I$(PLATFORM_SRC)/javavm/export \
	-I$(OPENWIN_HOME)/include 
endif

ifeq ($(CC_VERSION),gcc)
  CFLAGS  += -fPIC
else
  # Make sure this library is always -KPIC
  ifndef GLOBAL_KPIC
    CFLAGS += -KPIC
  endif 
endif

LDFLAGS += -L$(OPENWIN_LIB) -R$(OPENWIN_RUNTIME_LIB)
