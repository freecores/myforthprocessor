#
# @(#)Makefile	1.14 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the generic version of medialib
#

BUILDDIR = ../../..
PACKAGE = sun.awt.medialib
LIBRARY = mlib_image
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_c.gmk

#
# Rules
#
include $(BUILDDIR)/common/Library.gmk

#
# set compile options for solaris 32- or 64-bit build
#
ifeq ($(ARCH_DATA_MODEL), 64)
  CFLAGS += -DMLIB_OS64BIT
endif # ARCH_DATA_MODEL


ifneq ($(PLATFORM), windows)
ifneq ($(CC_VERSION), gcc)
  ifeq ($(ARCH), i586)
      _OPT = -xarch=386 -xchip=pentium -xO4 -xlibmil -nofstore \
	     -fsimple -xdepend -xrestrict
  else # ARCH
      _OPT = -xO4 -xlibmil -nofstore \
	     -fsimple -xdepend -xrestrict
  endif # ARCH
else # CC_VERSION
    _OPT = -O3
endif # CC_VERSION
endif # PLATFORM

ifneq ($(PLATFORM), sparc)
      OTHER_CFLAGS += -DMLIB_NO_LIBSUNMATH
endif

CPPFLAGS += \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/medialib 

OTHER_LDLIBS = $(LIBM) -ldl

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
