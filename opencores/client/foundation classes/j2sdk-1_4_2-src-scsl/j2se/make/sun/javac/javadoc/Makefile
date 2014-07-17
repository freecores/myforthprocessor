#
# @(#)Makefile	1.13 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the new javadoc tool.
#

BUILDDIR = ../../..
PRODUCT = sun
PACKAGE = com.sun.tools.javadoc
PROGRAM = javadoc
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

GIF_FILES = com/sun/tools/doclets/resources/inherit.gif
 
#
# Resources (must be made before .recompile).
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
	com/sun/tools/doclets/standard/resources/standard.properties \
	com/sun/tools/doclets/resources/doclets.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
all: classes gif_files

JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.javadoc.Main\" }"
include $(BUILDDIR)/common/Program.gmk

gif_files: $(CLASSBINDIR)/$(GIF_FILES)

$(CLASSBINDIR)/%.gif: $(SHARE_SRC)/classes/%.gif
	@$(MKDIR) -p $(@D)
	@$(RM) $@
	$(CP) $< $@

clean clobber::
	$(RM) -r $(CLASSBINDIR)/com/sun/tools/javadoc

JAVAC_CMD = $(JAVAC_BOOT_CMD)
