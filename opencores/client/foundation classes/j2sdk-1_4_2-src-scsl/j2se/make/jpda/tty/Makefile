#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 03/01/23
#
# Makefile for building the tty (aka JDB) on JDI
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.example.debug.tty
PRODUCT = jpda
PROGRAM = jdb
include $(BUILDDIR)/common/Defs.gmk

OTHER_INCLUDES = -I$(INCLUDEDIR) -I$(PLATFORM_INCLUDE)

JAVA_ARGS = "{\"com.sun.tools.example.debug.tty.TTY\"}"

#
# Files to compile.
#
include FILES_java.gmk

JAVA_SRC_DEST_DIR = $(GENSRCDIR)

SOURCEPATH = $(JAVA_SRC_DEST_DIR)$(CLASSPATH_SEPARATOR)$(VPATH.java)

#
# Resources
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_JAVA = $(TARGDIR)com/sun/tools/example/debug/tty/TTYResources.java
include $(BUILDDIR)/common/Resources.gmk

#
# Launcher source is in core
#
LAUNCHER_PLATFORM_SRC = $(PLATFORM_SRC)
LAUNCHER_SHARE_SRC = $(SHARE_SRC)

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk
