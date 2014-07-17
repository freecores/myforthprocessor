#
# @(#)Makefile	1.37 03/02/20
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


BUILDDIR = ../..
PACKAGE = java.util.zip
LIBRARY = zip
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

ZLIB_VERSION = 1.1.3

#
# Files to compile.
#
include FILES_c.gmk
include FILES_java.gmk

FILES_export = \
	java/util/zip/CRC32.java \
	java/util/zip/Adler32.java \
	java/util/zip/Inflater.java \
	java/util/zip/Deflater.java \
	java/util/zip/ZipFile.java \
	java/util/zip/ZipEntry.java \
	java/util/jar/JarFile.java

ifneq ($(PLATFORM), windows)
OTHER_CFLAGS += -DUSE_MMAP 
else 
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=2
endif

#
# Library to compile.
#
ifeq ($(PLATFORM), solaris)
  FILES_reorder += reorder-$(ARCH)
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Bootstrap stage.
#
JAVAC_CMD = $(JAVAC_BOOT_CMD)
JAVAH_CMD = $(JAVAH_BOOT_CMD)

ifeq ($(PLATFORM), windows)
OTHER_LCF = -export:ZIP_Open -export:ZIP_FindEntry -export:ZIP_ReadEntry \
	    -export:ZIP_GetNextEntry
else
CPPFLAGS += -UDEBUG 
endif

CPPFLAGS += -I$(SHARE_SRC)/native/java/util/zip/zlib-$(ZLIB_VERSION)

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(SHARE_SRC)/native/$(PKGDIR)/zlib-$(ZLIB_VERSION)

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

#
# Link to JVM library for JVM_Zip* functions
#
OTHER_LDLIBS = $(JVMLIB)
