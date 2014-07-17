#
# @(#)Makefile	1.26 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.26 03/01/23
#
# Makefile for building sun.tools.serialver.*
#

BUILDDIR = ../..
PACKAGE = sun.tools.serialver
PRODUCT = sun
PROGRAM = serialver
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
FILES_java = \
	sun/tools/serialver/SerialVer.java

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.serialver.SerialVer\" }"
include $(BUILDDIR)/common/Program.gmk

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = serialver.properties 
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/sun/tools/serialver
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/tools/serialver/resources
include $(BUILDDIR)/common/Resources.gmk
