#
# @(#)Makefile	1.9 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Transforms sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Transforms
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/Rotate.java \
             $(PKGPATH)/SelectTx.java \
             $(PKGPATH)/TransformAnim.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Transforms

include ../../DemoJava2D.gmk
