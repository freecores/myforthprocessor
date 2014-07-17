#
# @(#)Makefile	1.23 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.23 03/01/23
#
# Makefile for building the JDI interface and 
# front-end implementation
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.jdi
PRODUCT = jpda
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

GENJDWP = $(PKGDIR)/JDWP.java
FILES_java += $(GENJDWP)

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = com/sun/tools/jdi/resources/jdi.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

JAVA_SRC_DEST_DIR = $(GENSRCDIR)

SOURCEPATH = $(PLATFORM_SRC)/classes:$(JAVA_SRC_DEST_DIR):$(VPATH.java)

$(CLASSDESTDIR)/%.class: $(JAVA_SRC_DEST_DIR)/classes/%.java
	@$(ECHO) $? >> $(TMPDIR)/.classes.list
