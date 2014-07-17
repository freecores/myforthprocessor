# @(#)Makefile	1.27 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the new java compiler.
#

BUILDDIR = ../../..
PRODUCT = sun
PACKAGE = com.sun.tools.javac
PROGRAM = javac
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

#
# Resources (must be made before .recompile).
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
        com/sun/tools/javac/v8/resources/compiler.properties \
        com/sun/tools/javac/v8/resources/javac.properties \
	com/sun/tools/javadoc/resources/javadoc.properties	
RESOURCE_SRC_DIR = $(GENSRCDIR)
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.javac.Main\" }"
include $(BUILDDIR)/common/Program.gmk

clean clobber::
	$(RM) -r $(CLASSBINDIR)/com/sun/tools/javac

# Use bootstrap compiler.
JAVAC_CMD = $(JAVAC_BOOT_CMD) -source 1.4
