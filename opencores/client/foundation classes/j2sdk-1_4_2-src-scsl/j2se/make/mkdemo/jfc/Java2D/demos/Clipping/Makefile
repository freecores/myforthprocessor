#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Clipping sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Clipping
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/Areas.java \
             $(PKGPATH)/ClipAnim.java \
             $(PKGPATH)/Intersection.java \
             $(PKGPATH)/Text.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Clipping

include ../../DemoJava2D.gmk
