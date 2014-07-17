#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Arcs_Curves sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Arcs_Curves
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/Arcs.java \
             $(PKGPATH)/BezierAnim.java \
             $(PKGPATH)/Curves.java \
             $(PKGPATH)/Ellipses.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Arcs_Curves

include ../../DemoJava2D.gmk
