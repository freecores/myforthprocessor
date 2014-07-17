#
# @(#)Makefile	1.24 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
MOTIF_VERSION = 2
MOTIF_VERSION_STRING=21
LIB_LOCATION = $(LIBDIR)/$(LIBARCH)/motif$(MOTIF_VERSION_STRING)

PACKAGE = sun.awt
LIBRARY = mawt
LIBRARY_OUTPUT = motif21
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

ifdef BUILD_MOTIF21
  include $(BUILDDIR)/sun/awt/mawt.gmk
else
  all optimized debug clean clobber ::
  # do nothing case if older OS and new motif 2.1 not found in the boot dir
endif
