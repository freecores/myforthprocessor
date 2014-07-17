#
# @(#)Makefile	1.15 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.15	03/01/23
#
# Makefile for building the BDI interface
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.example.debug.bdi
PRODUCT = jpda
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

JAVA_SRC_DEST_DIR = $(GENSRCDIR)

SOURCEPATH = $(JAVA_SRC_DEST_DIR):$(VPATH.java)

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk
