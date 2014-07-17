#
# @(#)Makefile	1.10 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Paths sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Paths
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/Append.java \
             $(PKGPATH)/CurveQuadTo.java \
             $(PKGPATH)/FillStroke.java \
             $(PKGPATH)/WindingRule.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Paths

include ../../DemoJava2D.gmk

