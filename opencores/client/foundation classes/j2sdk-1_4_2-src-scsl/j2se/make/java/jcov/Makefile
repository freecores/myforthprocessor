#
# @(#)Makefile	1.12 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
LIBRARY = jcov
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_c.gmk

OTHER_INCLUDES = -I$(SHARE_SRC)/tools/jcov

ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS = wsock32.lib winmm.lib
  # Files built here do not compile with warning level 3
  COMPILER_WARNING_LEVEL=1
else # PLATFORM
  LIBSOCKET = -lsocket
  OTHER_LDLIBS = $(LIBSOCKET) -lnsl -ldl
endif
#
# Tell library.gmk to copy the txt file first
#
INIT = $(LIBDIR)/jvm.jcov.txt

OTHER_INCLUDES += -I$(SHARE_SRC)/javavm/include \
		  -I$(SHARE_SRC)/hpi/export \
		  -I$(PLATFORM_SRC)/javavm/include \
		  -I$(PLATFORM_SRC)/hpi/export

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
vpath %.c $(SHARE_SRC)/tools/jcov

#
# Install the jcov prelude
#
$(LIBDIR)/jvm.jcov.txt: $(SHARE_SRC)/tools/jcov/jvm.jcov.txt
	-$(RM) $@
	$(CP) $< $@

clean:: 
	$(RM) $(LIBDIR)/jvm.jcov.txt

