#
# @(#)Makefile	1.7 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.util.logging
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

#
# Rules
#
all: classes properties

#
# Files to compile.
#
include FILES_java.gmk

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = sun/util/logging/resources/logging.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

include $(BUILDDIR)/common/Classes.gmk

properties: $(LIBDIR) $(LIBDIR)/logging.properties

$(LIBDIR)/logging.properties: $(SHARE_SRC)/lib/logging.properties
	@$(RM) $@
	$(CP) $< $@

