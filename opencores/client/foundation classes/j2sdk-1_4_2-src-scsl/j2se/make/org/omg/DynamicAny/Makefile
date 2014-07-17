#
# @(#)Makefile	1.19 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# This makefile generates the classes defined in DynamicAny.idl.
# It doesn't override the existing ones because we are adding javadoc manually
# and therefore we have checked the files into the workspace.
# Ultimately we want to generate those files from idl with javadoc generated from
# idl comments.

BUILDDIR = ../../..
include $(BUILDDIR)/common/Defs.gmk

#
# What to compile?
#

PACKAGE = org.omg.DynamicAny
PRODUCT = org

#
# Files
#

include ../DynamicAny_java.gmk

FILES_java = $(DYNANY_GENERATED_java)

include $(BUILDDIR)/common/Classes.gmk
