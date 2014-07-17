#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.18 03/01/23
#
# Makefile for building the JDI back-end implementation
#

BUILDDIR = ../../..
LIBRARY = dt_socket
PRODUCT = jbug
include $(BUILDDIR)/common/Defs.gmk

ifeq ($(PLATFORM), windows)
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=0
endif

ifeq ($(PLATFORM), linux)
  LIBSOCKET = 
  OTHER_LDLIBS += -lnsl
endif

ifeq ($(PLATFORM), solaris)
  LIBSOCKET = -lsocket
  OTHER_LDLIBS += -lnsl -lsocket
endif

OTHER_INCLUDES = -I$(INCLUDEDIR) -I$(PLATFORM_INCLUDE) \
                 -I$(SHARE_SRC)/transport/export \
                 -I$(SHARE_SRC)/transport/socket \
                 -I$(SHARE_SRC)/back \
                 -I$(PLATFORM_SRC)/transport/socket \
                 -I$(PLATFORM_SRC)/back
#
# Files to compile.
#
##  include Exportedfiles.gmk
include FILES_c.gmk

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

# We don't want to link against -ljava
LDLIBS_OPT = 
LDLIBS_DBG = 

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(SHARE_SRC)/transport/socket:$(PLATFORM_SRC)/transport/socket



