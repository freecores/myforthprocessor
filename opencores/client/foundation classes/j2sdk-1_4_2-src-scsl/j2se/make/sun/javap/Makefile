#
# @(#)Makefile	1.15 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building javap
#

BUILDDIR = ../..
PACKAGE = sun.tools.javap
PRODUCT = sun
PROGRAM = javap
include $(BUILDDIR)/common/Defs.gmk

#
# Files.
#
include FILES_java.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.javap.Main\" }"
include $(BUILDDIR)/common/Program.gmk
