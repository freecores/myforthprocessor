#
# @(#)Makefile	1.17 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Stub for source preprocessing.
#

BUILDDIR = ../../..

include $(BUILDDIR)/common/Defs.gmk

JAVAC_PRE_SRC		= $(SHARE_SRC)/javac
JAVAC_JAVA_SRC		= $(GENSRCDIR)

TARGDIR	= $(JAVAC_JAVA_SRC)/
include $(BUILDDIR)/sun/javac/javac/FILES_java.gmk
JAVAC_SOURCES = $(FILES_java)

#
# Resources
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = \
        com/sun/tools/javac/v8/resources/compiler.properties \
        com/sun/tools/javac/v8/resources/javac.properties \
        com/sun/tools/javadoc/resources/javadoc.properties
RESOURCE_SRC_DIR  = $(JAVAC_PRE_SRC)
RESOURCE_DEST_DIR = $(JAVAC_JAVA_SRC)
include $(BUILDDIR)/common/Resources.gmk

.javac.resources: $(PFILES)

all optimized debug:: .javac.sources .javac.resources

.javac.sources: $(JAVAC_SOURCES)

$(JAVAC_JAVA_SRC)/%.java: $(JAVAC_PRE_SRC)/%.java
	-$(MKDIR) -p `$(DIRNAME) $@`
	$(RM) $@
	$(CP) $< $@

clean clobber::
	$(RM) -r $(JAVAC_JAVA_SRC)/com/sun/tools/javac

.PHONY: .javac.sources .javac.resources
