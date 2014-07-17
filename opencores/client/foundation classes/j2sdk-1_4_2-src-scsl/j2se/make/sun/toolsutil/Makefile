#
# @(#)Makefile	1.11 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.11 03/01/23
#
# Makefile for building util in tools
#

BUILDDIR = ../..
PACKAGE = sun.tools.util
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Directories
#
FILES_java = \
	sun/tools/util/ModifierFilter.java \
        sun/tools/util/CommandLine.java

FILES_class = $(FILES_java:%.java=$(CLASSBINDIR)/%.class)

#
# Rules
#
include $(BUILDDIR)/common/Rules.gmk

all install : classes

clean :: .delete.classlist
	$(RM) -r $(CLASSBINDIR)/sun/tools/util
