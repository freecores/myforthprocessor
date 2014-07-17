#
# @(#)Makefile	1.18 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building native2ascii
#

BUILDDIR = ../..
PACKAGE = sun.tools.native2ascii
PRODUCT = sun
PROGRAM = native2ascii
include $(BUILDDIR)/common/Defs.gmk

#
# Files.
#
FILES_java = \
	sun/tools/native2ascii/Main.java \
        sun/tools/native2ascii/A2NFilter.java \
        sun/tools/native2ascii/N2AFilter.java

#
# Resources
#
RESOURCE_BUNDLES_JAVA = $(TARGDIR)sun/tools/native2ascii/resources/MsgNative2ascii.java
LOCALE_SET_DEFINITION = j2sdk
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.native2ascii.Main\" }"
include $(BUILDDIR)/common/Program.gmk
