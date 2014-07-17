#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
HEADLESS = true
NO_ROBOT = true
MOTIF_VERSION = none
MOTIF_VERSION_STRING=none
LIB_LOCATION = $(LIBDIR)/$(LIBARCH)/headless

PACKAGE = sun.awt
LIBRARY = mawt
LIBRARY_OUTPUT = headless
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

ifdef BUILD_HEADLESS
  include $(BUILDDIR)/sun/awt/mawt.gmk
else
  all optimized debug clean clobber ::
  # do nothing case
endif

