#
# @(#)Makefile	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Fonts sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Fonts
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/AttributedStr.java \
             $(PKGPATH)/Highlighting.java \
             $(PKGPATH)/Outline.java \
             $(PKGPATH)/Tree.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Fonts

include ../../DemoJava2D.gmk
