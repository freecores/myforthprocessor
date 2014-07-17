#
# @(#)Makefile	1.27 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the old java compiler.
#

BUILDDIR = ../../..
PRODUCT	 = oldjavac

include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

#
# Resources (must be made before .recompile).
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = javac.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/sun/tools/javac/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/tools/javac/resources
include $(BUILDDIR)/common/Resources.gmk

all:	$(PFILES)

clean clobber::
	$(RM) -r $(CLASSBINDIR)/sun/tools/java $(CLASSBINDIR)/sun/tools/asm \
		 $(CLASSBINDIR)/sun/tools/tree
