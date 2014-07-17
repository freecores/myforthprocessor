#
# @(#)Makefile	1.51 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for appletviewer classes, wrappers, and resources.
#

BUILDDIR = ../..
PACKAGE = sun.applet
PRODUCT = sun
PROGRAM = appletviewer
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

#
# Resources
#
RESOURCE_BUNDLES_JAVA = $(TARGDIR)sun/applet/resources/MsgAppletViewer.java
LOCALE_SET_DEFINITION = j2re
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"sun.applet.Main\" }"
include $(BUILDDIR)/common/Program.gmk

ifneq ($(PLATFORM), windows)
#
# Anything with a GUI needs X11 to be linked in.
#
OTHER_LDLIBS += -L$(OPENWIN_LIB) -lX11 
endif # PLATFORM
