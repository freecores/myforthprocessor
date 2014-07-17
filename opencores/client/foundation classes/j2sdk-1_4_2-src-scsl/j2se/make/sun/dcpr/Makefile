#
# @(#)Makefile	1.22 03/03/19
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.dc
LIBRARY = dcpr
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_c.gmk
include FILES_java.gmk

FILES_export = \
	sun/dc/pr/PathDasher.java \
	sun/dc/pr/PathFiller.java \
	sun/dc/pr/PathStroker.java \

#
# Rules.
#
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/doe
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/path
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/pr
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/util

ifeq ($(PLATFORM), windows)
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=1
  LDLIBS += $(OBJDIR)/../../../sun.awt/awt/$(OBJDIRNAME)/awt$(SUFFIX).lib

  ifeq ($(VARIANT), DBG)
    ifdef MFC_DEBUG
      CFLAGS += -D_DEBUG
    endif # MFCDEBUG
  endif # VARIANT
else # PLATFORM
  OTHER_LDLIBS = $(LIBM) -lawt$(SUFFIX)
endif # PLATFORM
CPPFLAGS += \
	-I$(SHARE_SRC)/native/sun/java2d  \
	-I$(SHARE_SRC)/native/$(PKGDIR)/doe \
	-I$(SHARE_SRC)/native/$(PKGDIR)/path \
	-I$(SHARE_SRC)/native/$(PKGDIR)/util \

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
