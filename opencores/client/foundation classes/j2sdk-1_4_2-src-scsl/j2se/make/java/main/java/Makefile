#
# @(#)Makefile	1.23 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.23 03/01/23
#

BUILDDIR = ../../..
PROGRAM = java
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

ifeq ($(PLATFORM), solaris)
LDFLAGS += -R$(OPENWIN_LIB)
endif

