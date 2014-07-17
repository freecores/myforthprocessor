#
# @(#)Makefile	1.19 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
LIBRARY = hprof
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_c.gmk

OTHER_INCLUDES = -I$(PLATFORM_SRC)/tools/hprof \
	         -I$(SHARE_SRC)/tools/hprof
ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS = wsock32.lib winmm.lib
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=0
else
  LIBSOCKET = -lsocket
  OTHER_LDLIBS = $(LIBSOCKET) -lnsl -ldl
endif

#
# Tell library.gmk to copy the txt file first
#
INIT = $(LIBDIR)/jvm.hprof.txt

#
# Need jlong.h.
#
OTHER_INCLUDES += -I$(SHARE_SRC)/native/common -I$(PLATFORM_SRC)/native/common

ifeq ($(VARIANT), DBG)
OTHER_CPPFLAGS += -DHASH_STATS -DWATCH_ALLOCS
endif

#
# Library to compile.
#
include $(BUILDDIR)/common/Library.gmk

# We don't want to link against -ljava
LDLIBS_OPT = 
LDLIBS_DBG = 

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(SHARE_SRC)/tools/hprof:$(PLATFORM_SRC)/tools/hprof

#
# Install the hprof prelude
#
$(LIBDIR)/jvm.hprof.txt: $(SHARE_SRC)/tools/hprof/jvm.hprof.txt $(LIBDIR)
	-$(RM) $@
	$(CP) $< $@

clean:: 
	$(RM) $(LIBDIR)/jvm.hprof.txt

