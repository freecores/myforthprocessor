#
# @(#)Makefile	1.29 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.accessibility
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

ifeq ($(NOCORE),true)
ifeq ($(DEBUG_CLASSFILES),true)
JAVAC_DEBUG_FLAG = -g
endif
BINDIR=$(BOOTDIR)/bin
JAVAC_CMD  = $(LATEST_JDK12)/bin/javac $(JAVAC_DEBUG_FLAG) $(JAVAC_VMFLAGS_J) $(JAVAC_BOOTCLASSPATH_NAME) $(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(BOOTDIR)/jre/lib/rt.jar -sourcepath $(VPATH.java) -d $(CLASSDESTDIR)
endif

#
# Files
#
include FILES_java.gmk

#
# Rules
#

all: classes

clean:: classes.clean

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = com/sun/accessibility/internal/resources/accessibility.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk
LOCALE_SUFFIXES += en

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

FILES_class = $(FILES_java:%.java=$(CLASSBINDIR)/%.class)
FILES_source = $(FILES_java:%.java=$(SHARE_SRC)/classes/%.java)

SOURCEPATH = $(SHARE_SRC)/classes
