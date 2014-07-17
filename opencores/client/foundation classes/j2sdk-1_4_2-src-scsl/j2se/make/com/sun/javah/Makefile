#
# @(#)Makefile	1.21 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for javah.
#

BUILDDIR = ../../..
PACKAGE = com.sun.tools.javah
PRODUCT = sun
PROGRAM = javah
include $(BUILDDIR)/common/Defs.gmk

#
# Files.
#
include FILES_java.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"com.sun.tools.javah.Main\" }"

include $(BUILDDIR)/common/Program.gmk

#
# Resources.
#

RESOURCE_BUNDLES_PROPERTIES = l10n.properties

ifeq ($(PLATFORM), solaris)
OTHER_PROPERTIES = SunOS_sparc.properties SunOS_sparcv9.properties
endif 

# property files are at src/share/classes/com/sun/tools/javah/resources/
ifeq ($(PLATFORM), windows)
OTHER_PROPERTIES = win32_x86.properties
endif # PLATFORM

ifeq ($(PLATFORM), linux)
OTHER_PROPERTIES = Linux_ppc.properties Linux_sparc.properties
endif

LOCALE_SET_DEFINITION = j2sdk
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/com/sun/tools/javah/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/com/sun/tools/javah/resources
include $(BUILDDIR)/common/Resources.gmk
