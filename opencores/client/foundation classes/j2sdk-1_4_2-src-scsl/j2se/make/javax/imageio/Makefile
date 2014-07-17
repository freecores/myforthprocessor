#
# @(#)Makefile	1.4 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.4 03/01/23
#
# Makefile for building all of JIIO (Java Image I/O) API
#

BUILDDIR = ../..
PACKAGE = javax.imageio
PRODUCT = jiio
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
include FILES_java.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Rules that copy files.
#
OPTIMIZED += copy-files
DEBUG += copy-files

copy-files: $(FILES_copy)

#
# Clean up internal-use-only package.
#
clean::
	$(RM) -r $(CLASSDESTDIR)/javax/imageio
	$(RM) -r $(CLASSDESTDIR)/com/sun/imageio

#
# Declare extra phony targets.
#
.PHONY: copy-files
