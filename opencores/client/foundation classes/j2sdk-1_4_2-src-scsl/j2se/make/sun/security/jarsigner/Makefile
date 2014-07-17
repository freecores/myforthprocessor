#
# @(#)Makefile	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.10 03/01/23
#
# Makefile for building jarsigner utility.
#

BUILDDIR = ../../..
PACKAGE = sun.security.tools
PRODUCT = sun
PROGRAM = jarsigner
include $(BUILDDIR)/common/Defs.gmk

#
# Class to launch tool with.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.security.tools.JarSigner\" }"

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk
