#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building extcheck
#

BUILDDIR = ../../..
PACKAGE = com.sun.extcheck
PRODUCT = sun
PROGRAM = extcheck
include $(BUILDDIR)/common/Defs.gmk

#
# Files.
#
FILES_java = \
	com/sun/tools/extcheck/ExtCheck.java \
	com/sun/tools/extcheck/Main.java

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.extcheck.Main\" }"

include $(BUILDDIR)/common/Program.gmk

#
# Cleanliness is next to godliness.
#
clean::
	$(RM) -r $(CLASSBINDIR)/com/sun/tools/extcheck \
		$(CLASSBINDIR)/com/sun/extcheck




