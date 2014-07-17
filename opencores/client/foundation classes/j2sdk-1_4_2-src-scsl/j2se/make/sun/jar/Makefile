#
# @(#)Makefile	1.38 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jar utility.
#

BUILDDIR = ../..
PACKAGE = sun.tools
PRODUCT = sun
PROGRAM = jar
include $(BUILDDIR)/common/Defs.gmk

#
# Files.
#
include FILES_java.gmk

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = sun/tools/jar/resources/jar.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.jar.Main\" }"
include $(BUILDDIR)/common/Program.gmk
