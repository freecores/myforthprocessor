#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.6 03/01/23
#
# Makefile for building javaw.exe (for windows)
#

BUILDDIR = ../../..
PROGRAM = javaw
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

OTHER_CPPFLAGS += -DJAVAW
LDLIBS_COMMON +=  user32.lib

