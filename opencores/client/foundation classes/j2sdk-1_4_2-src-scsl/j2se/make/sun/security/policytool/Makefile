#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.15 03/01/23
#
# Makefile for building policytool utility.
#

BUILDDIR = ../../..
PACKAGE = sun.tools.security
PRODUCT = sun
PROGRAM = policytool
include $(BUILDDIR)/common/Defs.gmk

#
# Class to launch tools with.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.security.tools.PolicyTool\" }"

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

ifneq ($(PLATFORM), windows)
#
# Anything with a GUI needs X11 to be linked in.
#
OTHER_LDLIBS += -L$(OPENWIN_LIB) -lX11 
endif # PLATFORM
