#
# @(#)Makefile	1.29 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.awt
LIBRARY = jpeg
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_c.gmk
include FILES_java.gmk

FILES_export = \
	sun/awt/image/JPEGImageDecoder.java \
	sun/awt/image/codec/JPEGImageDecoderImpl.java \
	sun/awt/image/codec/JPEGImageEncoderImpl.java \
	com/sun/imageio/plugins/jpeg/JPEGImageReader.java \
	com/sun/imageio/plugins/jpeg/JPEGImageWriter.java

#
# Rules
#
ifeq ($(PLATFORM), solaris)
  FILES_reorder += reorder-$(ARCH)
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Add to ambient vpath to get files in a subdirectory
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/image/jpeg

#
# C Flags
#
CFLAGS += -DNIFTY
ifneq ($(PLATFORM), windows)
  ifneq ($(CC_VERSION),gcc)
    CFLAGS += -xCC
  endif # CC_VERSION
else # PLATFORM
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=0
endif # PLATFORM

CLASSES.export += java.io.InputStream

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
