#
# @(#)Makefile	1.12 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ..
JAVAWS_TOPDIR = ../..
DEPLOY_TOPDIR = ../..
include ../common/Defs.gmk

# debug
include ../common/JPDA.gmk

# File groupings
include FILES_java.gmk
include FILES_c.gmk
include FILES_resource.gmk

# Main Rules
ifeq ($(ARCH_DATA_MODEL),32)

dummy := $(shell $(MKDIR) -p $(OUTPUTDIR) $(TMPDIR))

ifndef EXTERNALSANITYCONTROL
OPTIMIZED += sanity-comprehensive
DEBUG += sanity-comprehensive
endif

OPTIMIZED += jump
DEBUG += jump

all:: optimized

clean::
	-$(RM) -r $(GENSRCBASE) $(CLASSDIR) $(I18NDESTDIR) \
		$(SERVLET_DESTDIR) $(OBJDIR) $(JARDIFF_DESTDIR)
	-$(RM) -r $(DEVBUNDLEDIR) $(SAMBUNDLEDIR) $(PROTO_DIR) $(IMAGE_DIR) \
		$(BUNDLE_DIR)/javaws*.* $(BUNDLE_DIR)/jinstall_javaws*.*

else # ARCH_DATA_MODEL

all::
clean::

endif # ARCH_DATA_MODEL

include ../common/Sanity.gmk
include ../common/Rules.gmk
include ../common/Release.gmk

.PHONY: all clean
