#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.13 03/01/23
#
# Makefile for building JDI shared memory transport implementation
#

BUILDDIR = ../../..
LIBRARY  = dt_shmem
PRODUCT = jbug
include $(BUILDDIR)/common/Defs.gmk

# 64-bit windows does not build at -W3
ifeq ($(PLATFORM), windows)
  ifeq ($(ARCH_DATA_MODEL), 64)
    COMPILER_WARNING_LEVEL=0
  endif
endif

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

include FILES_c.gmk

OTHER_INCLUDES = -I$(INCLUDEDIR) -I$(PLATFORM_INCLUDE) \
                 -I$(SHARE_SRC)/transport/export \
                 -I$(SHARE_SRC)/transport/shmem \
                 -I$(SHARE_SRC)/back \
                 -I$(PLATFORM_SRC)/transport/shmem \
                 -I$(PLATFORM_SRC)/back

#
# Stuff to feed into javah.
#
FILES_export = \
	com/sun/tools/jdi/SharedMemoryConnection.java \
	com/sun/tools/jdi/SharedMemoryTransport.java

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

#
# vpaths for the transport (should not be seen by others)
#
vpath %.c $(SHARE_SRC)/native/com/sun/tools/jdi:$(SHARE_SRC)/transport/shmem:$(PLATFORM_SRC)/transport/shmem
