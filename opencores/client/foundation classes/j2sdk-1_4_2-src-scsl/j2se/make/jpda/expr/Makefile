#
# @(#)Makefile	1.16 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.16 03/01/23
#
# Makefile for building client expression parsing code.
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.example.debug.expr
PRODUCT = jpda
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

FILES_java += $(GENFILES_java)

JAVA_SRC_DEST_DIR = $(GENSRCDIR)

SOURCEPATH = $(JAVA_SRC_DEST_DIR):$(VPATH.java)

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

$(CLASSDESTDIR)/%.class: $(JAVA_SRC_DEST_DIR)/classes/%.java
	@$(ECHO) $? >> $(TMPDIR)/.classes.list
