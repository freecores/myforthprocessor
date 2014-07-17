#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Images sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Images
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/DukeAnim.java \
             $(PKGPATH)/JPEGFlip.java \
             $(PKGPATH)/ImageOps.java \
             $(PKGPATH)/WarpImage.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Images

include ../../DemoJava2D.gmk
