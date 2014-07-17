#
# @(#)Makefile	1.13 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the VIS (solaris-only) version of medialib
#
BUILDDIR = ../../..
PACKAGE = sun.awt.medialib
LIBRARY = mlib_image_v
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

vpath %.s	$(PLATFORM_SRC)/native/sun/awt/medialib
vpath %.c	$(SHARE_SRC)/native/sun/awt/medialib

#
# set compile options for solaris 32- or 64-bit build
#
ifeq ($(ARCH_DATA_MODEL), 64)
  ARCH_GRAPHICS = -xarch=v9a 
  CFLAGS += -DMLIB_OS64BIT
  INLINE = $(PLATFORM_SRC)/native/sun/awt/medialib/vis_64.il
  _OPT = -xO4 -xdepend -xrestrict
else # ARCH_DATA_MODEL
  ARCH_GRAPHICS = -xarch=v8plusa
  INLINE = $(PLATFORM_SRC)/native/sun/awt/medialib/vis.il
  _OPT = -fsimple -dalign -xlibmil -xO4 -xdepend -xrestrict
endif # ARCH_DATA_MODEL

CFLAGS += $(ARCH_GRAPHICS) -xchip=ultra $(INLINE) -I$(PLATFORM_SRC)/native/sun/awt/medialib -I$(SHARE_SRC)/native/sun/awt/medialib

$(OBJDIR)/%.$(OBJECT_SUFFIX): %.s
	$(AS) -P $(ARCH_GRAPHICS) $(CC_OBJECT_OUTPUT_FLAG)$@ $<


OTHER_LDLIBS = $(LIBM) -ldl

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
