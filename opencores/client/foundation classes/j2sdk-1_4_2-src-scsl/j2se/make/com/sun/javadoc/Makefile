#
# @(#)Makefile	1.44 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building doclets
#

BUILDDIR = ../../..
PACKAGE = com.sun.javadoc
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files.
#
include $(BUILDDIR)/sun/javac/javadoc/FILES_java.gmk

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
        com/sun/tools/doclets/resources/doclets.properties \
        com/sun/tools/doclets/standard/resources/standard.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Cleanliness is next to godliness.
#
clean::
	$(RM) -r $(CLASSBINDIR)/com/sun/tools/doclets \
		$(CLASSBINDIR)/com/sun/javadoc

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk
